# FLY 项目回归检查

`check_standalone_projects.py` 通过公开的生成器和应用启动脚本，在临时目录中验证独立工程。工程所用的板卡和模板来自 `templates/`。

| 参数 | 覆盖 |
|---|---|
| 无 | FRDM-MCXN947 三个模板的生成、SDK 锁定安装、CMake 入口、编译、VS Code 配置和固件包 |
| `--matrix config` | 校验生成配置、架构与调度器，以及包清单发现和工具菜单的默认提供者规则 |
| `--matrix arm` | 构建每个 Cortex-M 板卡的裸机模板，并对三个 FRDM-MCXN947 模板执行完整检查 |
| `--matrix host` | 生成并构建当前宿主平台的最小工程 |
| `--matrix renode` | 为工具包声明的板卡与模板生成工程，编译打包并在真实 Renode 进程中验证最终 BIN、固件 CRC 与 RTT 启动日志 |
| `--build-type Release` | 矩阵和模板构建使用 Release，默认使用 Debug |
| `--sdk-setup-only` | 验证独立 Git 工程的 SDK 安装、版本更新和应用启动脚本 |
| `--image-links-only` | 验证现有镜像链接辅助函数 |

需要 Git、Python 3、`kconfiglib` 和 `requests`。构建检查还需要 CMake 和对应工具链。

仿真矩阵需要 Renode 1.17.0。可以用 `--renode /path/to/renode` 指定可执行文件。

```sh
python3 tool/test/check_standalone_projects.py --matrix config
python3 tool/test/check_standalone_projects.py --matrix arm
python3 tool/test/check_standalone_projects.py --matrix host
python3 tool/test/check_standalone_projects.py --matrix renode --build-type Release
```

动作协议、打包边界和缓存回归可单独运行。实板 J-Link 及 VS Code 调试验证在对应运行环境中执行。

```sh
python3 -m unittest discover -s tool/test -p 'test_project_actions.py'
python3 -m unittest discover -s tool/test -p 'test_image_payload.py'
```
