# FLY 项目回归检查

`check_standalone_projects.py` 通过公开的生成器和应用启动脚本，在临时目录中验证独立工程。工程所用的板卡和模板来自 `templates/`。

| 参数 | 覆盖 |
|---|---|
| 无 | FRDM-MCXN947 三个模板的生成、SDK 锁定安装、CMake 入口、编译、VS Code 配置和固件包 |
| `--matrix config` | 为所有受测板卡和模板生成配置，校验选中的架构、调度器和 Kconfig 警告 |
| `--matrix arm` | 构建每个 Cortex-M 板卡的裸机模板，并对三个 FRDM-MCXN947 模板执行完整检查 |
| `--matrix host` | 生成并构建当前宿主平台的最小工程 |
| `--build-type Release` | 矩阵和模板构建使用 Release，默认使用 Debug |
| `--sdk-setup-only` | 验证独立 Git 工程的 SDK 安装、版本更新和应用启动脚本 |
| `--image-links-only` | 验证固件包的当前目录链接 |

需要 Git、Python 3、`kconfiglib` 和 `requests`。构建检查还需要 CMake 和对应工具链。

```sh
python3 tool/test/check_standalone_projects.py --matrix config
python3 tool/test/check_standalone_projects.py --matrix arm
python3 tool/test/check_standalone_projects.py --matrix host
```
