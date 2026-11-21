## 变更内容

<!-- 一句话说明改了什么、为什么 -->

## 目标板 / 工程

- 应用或模板
- 应用使用的 `defconfig`
- 影响的 package：`package/<name>`（如有）

## 验证

- [ ] 在应用目录运行 `./build.sh loadconfig defconfig` 无 Kconfig warning
- [ ] 在应用目录运行 `./build.sh build` 通过
- [ ] `python3 tool/test/check_standalone_projects.py --matrix config`
- [ ] 运行时证据（RTT 日志 / 串口输出截图，shell、网络、timer 类改动必填）：

```
粘贴关键日志
```

## 影响面

- [ ] Kconfig 改动与 defconfig 同步（含 `saveconfig` 重新生成的文件）
- [ ] 新增模板或板卡已加入生成与构建回归
- [ ] 新增工程所需的宿主依赖和工具链已在 `docs/CI.md` 说明

## 备注

<!-- 未验证的点、已知限制、需要 reviewer 特别关注的地方 -->
