## 变更内容

<!-- 一句话说明改了什么、为什么 -->

## 目标板 / 工程

- project：`project/<arch>/<demo>`
- defconfig：`project/<arch>/<demo>/defconfig`
- 影响的 package：`package/<name>`（如有）

## 验证

- [ ] `./build.sh loadconfig <defconfig>` 无 Kconfig warning
- [ ] `./build.sh build` 通过
- [ ] `python3 tool/test/check_freertos_config.py`
- [ ] `python3 tool/test/check_kconfig_hygiene.py`
- [ ] 运行时证据（RTT 日志 / 串口输出截图，shell、网络、timer 类改动必填）：

```
粘贴关键日志
```

## 影响面

- [ ] Kconfig 改动与 defconfig 同步（含 `saveconfig` 重新生成的文件）
- [ ] 新增 project 已加入 `tool/test/check_build_arm.py` 的工程列表
- [ ] 新增工程所需的 host 依赖 / 工具链已在 `docs/CI.md` 的已知限制里说明

## 备注

<!-- 未验证的点、已知限制、需要 reviewer 特别关注的地方 -->
