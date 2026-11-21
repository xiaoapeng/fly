# CI 流程

CI 在临时目录中用 `fly.sh new` 或 `fly.bat new` 生成应用，再从应用根目录运行构建。固件包和已拉取的依赖保存在各自应用的构建目录。

## 检查与触发

| 流水线 | 触发 | 内容 |
|---|---|---|
| `.github/workflows/ci.yml` | PR、push 到 `master`、手动 | 生成配置、ARM Debug 编译和打包、三平台原生构建 |
| `.github/workflows/nightly.yml` | 每日 02:17 UTC、手动 | 相同的 ARM 工程和固件包使用 Release 构建 |

快速 CI 在同一分支有新提交时会取消上一次未完成的运行。

## 必需检查

| 检查名 | Runner | 覆盖 |
|---|---|---|
| `kconfig` | Ubuntu x64 | 为受测板卡的各模板组合生成工程，确认配置无警告并选中对应架构和调度器 |
| `arm-build` | Ubuntu x64 | 构建七类 Cortex-M 架构对应的最小工程。FRDM-MCXN947 的三个模板还检查 SDK 锁定安装、VS Code 配置和固件打包 |
| `host-linux` | Ubuntu x64 | 用系统 GCC 构建生成的 Linux 应用 |
| `host-macos` | macOS arm64 | 用 Apple Clang 构建生成的 macOS 应用 |
| `host-windows` | Windows x64 | 用 MinGW-w64 构建生成的 Windows 应用，另测批处理入口、SDK 锁定安装和 NTFS 目录链接 |

ARM 覆盖 MCXN947、STM32F0/F1/F4/H7、GD32F10x 和 GD32E23x。CH58x 和 GD32VF103 的 RISC-V 交叉编译暂不纳入 CI。配置检查会对照 `arch/Kconfig` 确认每个受测架构都有裸机工程。

FreeRTOS 和 nanosig 等外部包在 CMake 配置时拉取，保存到各应用的 `build/deps/`。CI 的打包检查验证应用根目录 `image/` 下的固件产物和 `image/CURRENT/` 链接，并确认 `distclean` 后仍可访问固件包。实板烧录与 RTT 运行需要单独验证。

## 分支保护（必需检查 + rebase 合并）

CI 文件本身不会阻止合并，必须在仓库设置里把这些检查设成 required。一次性配置：

```sh
# 必需检查 + 严格模式（分支落后 master 时必须先 rebase 再跑一遍）
gh api -X PUT "repos/:owner/:repo/branches/master/protection" --input - <<'JSON'
{
  "required_status_checks": {
    "strict": true,
    "contexts": ["kconfig", "arm-build", "host-linux", "host-macos", "host-windows"]
  },
  "enforce_admins": true,
  "required_pull_request_reviews": null,
  "restrictions": null,
  "allow_force_pushes": false,
  "allow_deletions": false
}
JSON

# 合并方式：只允许 Rebase and merge；合并后自动删分支
gh api -X PATCH "repos/:owner/:repo" \
  -F allow_rebase_merge=true \
  -F allow_squash_merge=false \
  -F allow_merge_commit=false \
  -F delete_branch_on_merge=true
```

`contexts` 里的名字必须和 job 的 `name:` 完全一致（写成 `kconfig`、`arm-build` 等）；改 job 名要同步改分支保护，否则检查永远不会被要求，等于没设。

## PR 规则

1. 改动走特性分支 + PR，**不直接 push `master`**（push 到 master 只作为事后兜底，不是交付路径）
2. 5 个必需检查全绿才能合并；`strict: true` 时还需先与 `master` 同步
3. 合并统一用 **Rebase and merge**（`gh pr merge --rebase`），保持 `master` 线性、无 merge 节点
4. 合并后删除特性分支
5. PR 描述按 `.github/pull_request_template.md` 填：目标工程、defconfig、构建命令、运行时证据（RTT 日志截图）、未验证点

## 本地复现

```sh
# 需要 Python、Git、kconfiglib 和 requests
python3 tool/test/check_standalone_projects.py --matrix config

# 需要 CMake 和 arm-none-eabi 工具链
python3 tool/test/check_standalone_projects.py --matrix arm
python3 tool/test/check_standalone_projects.py --matrix arm --build-type Release

# 用当前宿主平台的原生工具链
python3 tool/test/check_standalone_projects.py --matrix host

# 只生成、编译和打包三个 FRDM-MCXN947 模板
python3 tool/test/check_standalone_projects.py
```

新增板卡时，在 `templates/boards/` 中登记板卡和支持的模板。检查从 `board.json` 发现受测配置，并为各受测架构验证裸机模板覆盖。

- [回归脚本说明](../tool/test/README.md)
- [Kconfig 规则](reference/kconfig-rules.md)
