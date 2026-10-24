# CI 流程

FLY 的持续集成分两条流水线：快速 CI（`ci.yml`，PR / push 到 `master` / 手动）和夜间 CI（`nightly.yml`，定时 + 手动）。设计原则与 nanosig 上游一致：**PR 路径只跑能快速给出结论的检查**，慢的 Release 全量构建和打包冒烟放夜间。

## 总览

| 流水线 | 文件 | 触发 | 作用 |
|---|---|---|---|
| 快速 CI | `.github/workflows/ci.yml` | `pull_request`、push 到 `master`、`workflow_dispatch` | 配置扫描 + 10 个 ARM 工程 Debug 构建 + 三平台宿主工程构建 |
| 夜间 CI | `.github/workflows/nightly.yml` | `schedule`（`17 2 * * *` UTC）+ `workflow_dispatch` | 全部 ARM 工程 Release 构建 + `make_img` 打包冒烟 |

同一分支的新 push 会取消上一次未完成的 CI（`concurrency` + `cancel-in-progress`）。

## 快速 CI 的五个 job（全部为必需检查）

| 检查名 | runner | 内容 | 超时 |
|---|---|---|---|
| `kconfig` | ubuntu-latest | `tool/test/check_freertos_config.py`（scheduler 选路）+ `tool/test/check_kconfig_hygiene.py`（choice 成员禁止 select、所有 defconfigs 零警告） | 10 min |
| `arm-build` | ubuntu-latest | `tool/test/check_build_arm.py`：10 个 arm-none-eabi 工程逐个 loadconfig + 全量重建 + 符号断言（FreeRTOS API 无 undefined、legacy 工程无 FreeRTOS 符号、`SysTick_Handler` 唯一） | 60 min |
| `host-linux` | ubuntu-latest | `tool/test/check_build.py --platform linux`：`project/linux/ehip-test` 干净重建 + 产物断言 | 20 min |
| `host-macos` | macos-latest | `tool/test/check_build.py --platform macos`：`project/macos/ehip-test`（Apple Clang + lld） | 20 min |
| `host-windows` | windows-latest | `tool/test/check_build.py --platform windows`：`project/windows/ehip-test` + `co-ctx-test`（MinGW-w64 GCC） | 30 min |

三个 `host-*` job 的构建逻辑都在 `tool/test/check_build.py` 里，平台 → 工程清单是该脚本的数据表；CI 只负责准备工具链然后调用它。脚本会先 `loadconfig`、删 `build/`、构建，再断言可执行文件存在（构建失败本身已经通过退出码判死，产物断言防的是"旧产物掩盖坏链接"）。

要点：

- **ARM 交叉编译只在 ubuntu 跑一次**。交叉结果与宿主无关，重复在 macOS/Windows 上跑只增加 flake 面；三平台各自跑的是**只有该平台能编**的宿主工程，这才是真正区分平台的部分。
- **Windows 必须 GNU 工具链**。`arch/x64/windows` 的 `gcc-host-windows` 工具链文件和工程里的 GNU 专有 flag（如 `-mpreferred-stack-boundary=4`）都不接受 MSVC / clang-cl。CI 用 runner 镜像自带的 MSYS2（`C:\msys64`）安装 `mingw-w64-x86_64-gcc`。
- **CI 用 `.venv` 固定解释器**。`cmake/python3.cmake` 优先使用仓库根的 `.venv`，否则 `find_package(Python3)` 可能选到没有 `requests` 的系统 Python（macOS runner 上就踩到了：`fetch_git_project.py` 直接 ImportError）。所以每个构建 job 都先 `python3 -m venv .venv && .venv/bin/python -m pip install kconfiglib requests`，让 CMake 与 `build.sh` 用同一个解释器。
- `check_build_arm.py` 每个工程前会删 `build/`，所以 CI 里每个工程都是干净重建，不吃缓存假阳性。
- 依赖包（`eventhub-os`、`ehip`、`ehip-tools`、`ehshell`、`freertos-kernel`、`nanosig` …）在 configure 阶段由 `fetch_git_project` 从 GitHub 拉取；CI 依赖公网访问，无需额外配置。`dl/` 缓存目前没做（后续可加，注意 key 要带 defconfig 里的 pin）。
- **本地 `dl/` 会漂移**。`ehip`/`ehip-tools`/`ehshell`/`eventhub` 的 `GIT_TAG` 是 `main`/`master` 加 `AUTO_UPDATE OFF`，所以本地克隆不会自动跟随上游：CI 新克隆拿的是最新 `main`，本地拿的是上次 fetch 的快照。本地构建失败而 CI 通过（或反之）时，先跑 `./build.sh package_update <name>`（`-l` 看列表）把包装到最新，再复现。彻底解决办法是把这些包也钉到 SHA，见"已知限制"。

### `host-macos` / `host-windows` 曾经不是必需检查（已修复）

这两个 job 最初上线时是红的，原因都在上游被 fetch 的包里，不在 FLY 代码：

- **`host-macos`**：`eventhub` 的 `eh_event_flags.c` 把普通 `eh_flags_t`（`typedef size_t`）传给 `atomic_load_explicit` 等 C11 泛型原子函数。标准要求实参是 `_Atomic` 指针，Apple Clang 21 严格报错（GCC 经 `__atomic_*` 放行，只是实现宽容）。
- **`host-windows`**：`ehshell` 在 MinGW 下有两处隐式声明 —— `bzero`（MinGW 不提供）与缺 `<ctype.h>` 的 `isprint`；GCC 14 起 C99+ 把隐式声明默认当错误，Linux 上的 GCC 13 只当警告。

已在上游修掉并推送：

| 仓库 | commit | 改动 |
|---|---|---|
| `xiaoapeng/eventhub_os` | `96916a7` | `eh_atomic.h` 的宏改用 `__atomic_*` 内建，不改字段类型、GCC/Clang 语义一致 |
| `xiaoapeng/ehshell` | `a1c1bcf` | `bzero` → `memset`；补 `<ctype.h>` 并把 `isprint` 实参转 `unsigned char` |

现在两个 job 与其余三项一样是必需检查。macOS runner 额外装 `lld`，因为 `cmake/tool_chain/clang-host-darwin.cmake` 固定加了 `-fuse-ld=lld`。

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
# 与 kconfig job 一致
python3 tool/test/check_freertos_config.py
python3 tool/test/check_kconfig_hygiene.py

# 与 arm-build job 一致（需要 PATH 里有 arm-none-eabi-gcc）
python3 tool/test/check_build_arm.py -j 4 --verbose

# 与 host-* job 一致（平台必须与宿主一致）
python3 tool/test/check_build.py --platform macos
python3 tool/test/check_build.py --platform linux

# 与 nightly/arm-release 一致：Release 全量
for dc in project/*/*/defconfig; do
  case "$dc" in project/linux/*|project/macos/*|project/windows/*) continue;; esac
  case "$dc" in project/ch58x/*|project/gd32vf103x/*) continue;; esac
  ./build.sh loadconfig "$dc" && ./build.sh build || break
done

# 手动触发夜间流水线（验证特性分支）
gh workflow run nightly.yml --ref <branch>
gh run watch
```

## 新增工程时要做什么

1. `tool/test/check_build_arm.py` 的 `ARM_PROJECTS` 列表加入 `(defconfig 路径, 可执行文件名, project id)`——CI 的 `arm-build` 直接读这个列表
2. 若是宿主工程，把 `(defconfig 路径, 可执行文件名)` 加进 `tool/test/check_build.py` 的 `PLATFORM_PROJECTS`——`host-*` job 直接读这张表
3. 需要新工具链的工程（如 RISC-V）先在 `docs/CI.md` 的已知限制里登记，别让它静默不被覆盖

## 已知限制

| 项 | 说明 |
|---|---|
| RISC-V 工程（`ch58x/ch582-demo`、`gd32vf103x/gd32vf103c-demo`） | 未纳入 CI。需要 `riscv-none-elf-`（xPack）与 `riscv-none-embed-`（MounRiver）两套交叉工具链，apt/Homebrew 都没有；`check_build_arm.py` 与夜间流水线都显式跳过 |
| `macos/ehip-test` 的 Debug 配置 | 工程 Debug flag 带 `-fsanitize=address,leak`；CI 只跑默认 Release，ASAN 路径未在 CI 中执行 |
| Windows job | 依赖 runner 镜像的 `C:\msys64`。镜像若更换路径，改 `host-windows` 的 PATH 注入即可 |
| `host-macos` 的工具链 | `clang-host-darwin.cmake` 固定 `-fuse-ld=lld`，机器上没有 `ld.lld` 时链接直接失败（报 `invalid linker name`）。CI 里 `brew install lld`；本地需要 `brew install lld` |
| `ehip` / `ehip-tools` / `ehshell` / `eventhub` 的版本漂移 | 四个包的 `GIT_TAG` 是浮动的 `main`/`master` + `AUTO_UPDATE OFF`：本地 `dl/` 不会自动更新，CI 新克隆永远拿最新。二者不一致时会出现"本地炸、CI 过"。短期用 `./build.sh package_update <pkg>` 对齐；长期应像 `nanosig` 一样钉到 SHA（未做，改动会牵动 4 个包 + 三平台验证） |
| 硬件行为 | CI 只做编译期与静态断言，Flash / RTT 运行证据仍需本地 `./build.sh flash` + `./build.sh rttlog`，PR 模板里要求人工附上 |
| 首次落地 | 本流水线**尚未在真实 runner 上跑过**。建议先推到特性分支观察一轮，再开启分支保护，避免把不稳定的 job 设成必需检查 |

## 相关文档

- [Kconfig 硬性规则](reference/kconfig-rules.md) — `kconfig` job 强制的规则来源
- [RTOS 选择语义](reference/rtos-choice.md)
- [tool/test/ 说明](../tool/test/README.md)
