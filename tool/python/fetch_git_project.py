#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import argparse
import json
import time
import concurrent.futures
import urllib.parse
import requests


class GitProjectFetcher:
    def __init__(self, source_dl_dir=None, package_info_file=None):
        self.source_dl_dir = source_dl_dir
        self.package_info_file = package_info_file
        self._check_git_installed()
        
    def _check_git_installed(self):
        """检查Git是否安装"""
        try:
            subprocess.run(['git', '--version'], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("[fetch_git_project] Git not found", file=sys.stderr)
            sys.exit(1)
    
    def fetch_git_project(self, git_project_name, git_repository, git_tag, deep_clone=False, auto_update=False):
        """拉取或更新Git仓库到指定目录"""
        # 检查必填参数
        if not all([git_project_name, git_repository, git_tag, self.source_dl_dir]):
            print("[fetch_git_project] Missing required parameters", file=sys.stderr)
            return False
        
        # 确保下载目录存在
        os.makedirs(os.path.dirname(self.source_dl_dir), exist_ok=True)
        
        # 检查源码和.git目录是否存在
        if not os.path.exists(self.source_dl_dir) or not os.path.exists(os.path.join(self.source_dl_dir, '.git')):
            print(f"[fetch_git_project] Cloning {git_repository} to {self.source_dl_dir}")
            
            # 构建克隆命令
            clone_cmd = ['git', 'clone']
            if not deep_clone:
                clone_cmd.extend(['--depth', '1'])
            clone_cmd.extend(['--branch', git_tag, git_repository, self.source_dl_dir])
            
            # 执行克隆命令
            try:
                subprocess.run(clone_cmd, check=True)
                print(f"[fetch_git_project] Cloned {git_project_name} successfully")
            except subprocess.CalledProcessError as e:
                print(f"[fetch_git_project] Failed to clone {git_repository}: {e}", file=sys.stderr)
                return False
        else:
            print(f"[fetch_git_project] {git_project_name} already cloned to {self.source_dl_dir}")
            
            if auto_update:
                print(f"[fetch_git_project] Updating {git_project_name}...")
                
                # 拉取最新内容
                try:
                    subprocess.run(
                        ['git', '-C', self.source_dl_dir, 'fetch', '--all', '--tags'],
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE
                    )
                except subprocess.CalledProcessError:
                    print(f"[fetch_git_project] Failed to fetch updates", file=sys.stderr)
                    return True  # 返回True，因为仓库已经存在，只是更新失败
                
                # 检查是否为tag
                is_tag = False
                try:
                    tag_output = subprocess.run(
                        ['git', '-C', self.source_dl_dir, 'tag', '--list', git_tag],
                        capture_output=True, text=True
                    ).stdout.strip()
                    is_tag = len(tag_output) > 0
                except subprocess.CalledProcessError:
                    pass
                
                if is_tag:
                    # tag通常是静态版本，不自动更新
                    print(f"[fetch_git_project] {git_tag} is a tag — no auto-update applied")
                else:
                    # 分支 → 自动拉取最新
                    print(f"[fetch_git_project] Switching to {git_tag} and pulling latest")
                    try:
                        # 切换到指定分支
                        subprocess.run(
                            ['git', '-C', self.source_dl_dir, 'checkout', git_tag],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE
                        )
                        # 拉取最新内容
                        subprocess.run(
                            ['git', '-C', self.source_dl_dir, 'pull'],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE
                        )
                        print(f"[fetch_git_project] Updated {git_project_name} successfully")
                    except subprocess.CalledProcessError as e:
                        print(f"[fetch_git_project] Failed to update branch {git_tag}: {e}", file=sys.stderr)
                        return True  # 返回True，因为仓库已经存在，只是更新失败
        
        # 创建package_info.txt文件
        if self.package_info_file:
            # 确保package_info_file的目录存在
            os.makedirs(os.path.dirname(self.package_info_file), exist_ok=True)
            current_package_info = f"{git_project_name}:\n    tag: {git_tag}\n    remote: {git_repository}\n"
            try:
                with open(self.package_info_file, 'w') as f:
                    f.write(current_package_info)
                print(f"[fetch_git_project] Created package info file: {self.package_info_file}")
            except Exception as e:
                print(f"[fetch_git_project] Failed to create package info file: {e}", file=sys.stderr)
        
        return True


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
    parser = argparse.ArgumentParser(description='Fetch Git projects')
    
    # 添加所有必要的参数
    parser.add_argument('--project-name', required=True, help='Name of the project')
    parser.add_argument('--repository', required=True, help='Git repository URL')
    parser.add_argument('--tag', required=True, help='Git tag or branch')
    parser.add_argument('--source-dl-dir', required=True, help='Source download directory (from CMAKE)')
    parser.add_argument('--package-info-file', required=True, help='Path to package info file (from CMAKE)')
    parser.add_argument('--deep-clone', action='store_true', help='Perform a deep clone')
    parser.add_argument('--auto-update', action='store_true', help='Auto update if already cloned')
    parser.add_argument('--mirror-source-json-file', required=True, help='Path to mirror source JSON file')
    
    args = parser.parse_args()
    
    # 初始化镜像源管理器并测试找到最佳镜像源
    mirror_manager = MirrorSourceManager(args.mirror_source_json_file)
    # 使用force_update参数调用find_best_mirror_source
    mirror_manager.find_best_mirror_source()
    
    # 使用最佳镜像源包装仓库URL
    mirrored_repository = mirror_manager.wrap_url_with_mirror(args.repository)
    
    fetcher = GitProjectFetcher(args.source_dl_dir, args.package_info_file)
    success = fetcher.fetch_git_project(
        args.project_name,
        mirrored_repository,  # 使用镜像后的URL
        args.tag,
        args.deep_clone,
        args.auto_update
    )
    
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()