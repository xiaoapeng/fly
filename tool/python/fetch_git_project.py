#!/usr/bin/env python3
"""Resolve and cache Git sources and manage the configured download mirrors."""

import argparse
import concurrent.futures
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import urllib.parse

import requests

from git_source_cache import GitSourceCache, write_if_changed


class MirrorSourceManager:
    def __init__(self, json_file_path):
        """初始化镜像源管理器"""
        self.json_file_path = json_file_path
        # 只有当文件存在时才加载数据，不存在时不打印信息
        self.mirror_data = []
        if os.path.exists(json_file_path):
            self.mirror_data = self._load_mirror_data()
    
    def _load_mirror_data(self):
        """加载镜像源JSON数据"""
        try:
            with open(self.json_file_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except Exception:
            return []
    
    def _save_mirror_data(self):
        """保存镜像源JSON数据"""
        try:
            with open(self.json_file_path, 'w', encoding='utf-8') as f:
                json.dump(self.mirror_data, f, ensure_ascii=False, indent=4)
            return True
        except Exception:
            # 保存失败时不打印信息
            return False
    
    def _test_mirror_source(self, mirror_source, test_url, timeout=3):
        """测试单个镜像源的延迟"""
        if not mirror_source.endswith('/'):
            mirror_source = mirror_source + '/'
        
        # 构建测试URL，格式为: https://mirror-url/https://original-url?t=timestamp
        timestamp = int(time.time() * 1000)
        mirrored_url = f"{mirror_source}{test_url}?t={timestamp}"
        
        try:
            start_time = time.time()
            response = requests.get(mirrored_url, timeout=timeout, stream=True)
            if response.status_code != 200:
                return mirror_source, float('inf')
            # 只读取一小部分数据来测试连接
            response.raw.read(10)
            response.close()
            elapsed_time = (time.time() - start_time) * 1000  # 转换为毫秒
            return mirror_source, elapsed_time
        except Exception:
            # 测试失败，返回高延迟
            return mirror_source, float('inf')
    
    def find_best_mirror_source(self, force_update=False, domain=None):
        """测试镜像源，找到延迟最低的那个
        
        Args:
            force_update (bool): 是否强制更新最佳镜像源
            domain (str, optional): 要测试的域名，如果为None则测试所有域名
            
        Returns:
            dict or str: 如果指定了domain则返回该域名的最佳镜像源，否则返回包含所有域名最佳镜像源的字典
        """
        if not self.mirror_data or len(self.mirror_data) == 0:
            return None
        
        results = {}
        
        # 直接遍历所有域名配置，在循环中同时完成判断和匹配
        for domain_config in self.mirror_data:
            current_domain = domain_config.get('domain', 'unknown')
            
            # 检查是否需要处理当前域名配置
            # 1. 如果指定了domain参数，只处理匹配的域名
            # 2. 无论是否指定domain，都只处理启用的域名配置（enable为True）
            if (domain and domain != current_domain) or not domain_config.get('enable', False):
                continue
            
            # 如果force_update为False且已经有最佳镜像源，则直接使用缓存
            best_mirror = domain_config.get('mirror-best')
            if not force_update and best_mirror and len(best_mirror) > 0:
                print(f"[fetch_git_project] Using cached best mirror for {current_domain}: {best_mirror}")
                results[current_domain] = best_mirror
                continue
            
            # 获取测试URL和镜像源列表
            test_url = domain_config.get('test-url')
            mirror_sources = domain_config.get('mirror-source-list', [])
            if not test_url or not mirror_sources:
                continue
            
            # 使用线程池并行测试所有镜像源
            best_delay = float('inf')
            domain_best_mirror = None
            
            with concurrent.futures.ThreadPoolExecutor(max_workers=min(40, len(mirror_sources))) as executor:
                # 提交所有测试任务
                futures = {executor.submit(self._test_mirror_source, source, test_url): source for source in mirror_sources}
                
                # 收集结果
                for future in concurrent.futures.as_completed(futures):
                    source, delay = future.result()
                    print(f"[fetch_git_project] Test mirror source {source} for {current_domain} delay: {delay:.2f}ms")
                    if delay < best_delay:
                        best_delay = delay
                        domain_best_mirror = source
                        # if best_delay < 800:  # 延迟小于800ms时提前终止
                        #     break
            
            # 更新该域名的最佳镜像源
            if domain_best_mirror:
                print(f"[fetch_git_project] Best mirror source for {current_domain}: {domain_best_mirror} with delay: {best_delay:.2f}ms")
                domain_config['mirror-best'] = domain_best_mirror
                results[current_domain] = domain_best_mirror
        
        # 保存所有域名配置的更新
        if results:
            self._save_mirror_data()
        
        # 根据输入参数返回相应的结果
        if domain:
            # 检查是否找到指定域名的结果
            if domain in results:
                return results[domain]
            # 如果指定了域名但没有处理（可能是禁用状态或没有匹配的配置），返回None
            return None
        else:
            # 如果没有指定域名但处理了某些域名配置，返回结果字典
            # 如果没有处理任何域名配置（全部被过滤掉），返回None
            return results if results else None
    
    def wrap_url_with_mirror(self, original_url):
        """使用最佳镜像源包装原始URL"""
        # 解析原始URL的域名
        parsed_url = urllib.parse.urlparse(original_url)
        original_domain = parsed_url.netloc
        
        # 查找匹配的域名配置
        matched_config = None
        for config in self.mirror_data:
            if config.get('domain') == original_domain and config.get('enable', False):
                matched_config = config
                break
        
        if not matched_config:
            return original_url
        
        # 获取最佳镜像源
        best_mirror = matched_config.get('mirror-best')
        if not best_mirror:
            return original_url
        
        # 确保镜像源URL以/结尾
        if not best_mirror.endswith('/'):
            best_mirror = best_mirror + '/'
        
        # 包装原始URL
        mirrored_url = f"{best_mirror}{original_url}"
        return mirrored_url


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-name", required=True)
    parser.add_argument("--repository", required=True)
    parser.add_argument("--tag", required=True, help="Git commit, tag or branch")
    parser.add_argument("--cache-dir", required=True, type=Path)
    parser.add_argument("--result-file", required=True, type=Path)
    parser.add_argument("--package-info-file", required=True, type=Path)
    parser.add_argument("--mirror-source-json-file", required=True, type=Path)
    parser.add_argument("--patch-dir", type=Path)
    parser.add_argument("--project-patch-dir", type=Path)
    parser.add_argument("--sparse-path", action="append", default=[])
    parser.add_argument("--deep-clone", action="store_true")
    parser.add_argument("--auto-update", action="store_true", help="Refresh a branch or tag")
    args = parser.parse_args()

    manager = MirrorSourceManager(args.mirror_source_json_file)
    patches = [directory for directory in (args.patch_dir, args.project_patch_dir) if directory is not None]
    try:
        result = GitSourceCache(args.cache_dir).fetch(
            args.project_name, args.repository, args.tag,
            result_file=args.result_file, patch_dirs=patches, sparse_paths=args.sparse_path,
            deep=args.deep_clone, refresh=args.auto_update, mirror=manager.wrap_url_with_mirror,
        )
        info = (f"{args.project_name}:\n"
                f"    ref: {args.tag}\n"
                f"    commit: {result['commit']}\n"
                f"    remote: {args.repository}\n"
                f"    source: {result['source_dir']}\n")
        if result["patch_id"]:
            info += f"    patches: {result['patch_id']}\n"
        write_if_changed(args.package_info_file, info)
        print(f"[fly_fetch_git_project] {args.project_name} {result['commit']} → {result['source_dir']}")
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f"[fly_fetch_git_project] {args.project_name}: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
