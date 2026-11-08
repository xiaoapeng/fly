# MCXN947 裸机按键与 RGB 灯效

复刻相邻 freertos-demo 的业务：nsbtn/lwbtn 扫描两个按键，以 nanosig 信号驱动 RGB 灯效。
板级时钟、GPIO、LED 驱动和链接脚本沿用同一份板级配置。

| 输入 | 行为 |
|---|---|
| 上电 | 红色、1 Hz 闪烁 |
| SW1 单击（板载 SW3，P0_6） | 红、绿、蓝、黄、品红、青、白循环 |
| SW1 双击 | 1、2、5 Hz 循环 |
| SW2 单击（P0_23） | 总开关：关闭全灭，开启后恢复所选颜色和频率 |
| 三连击及以上 | 不改变灯效 |

```sh
./build.sh loadconfig project/mcxn947/baremetal-demo/defconfig
./build.sh build Debug -j 8
./build.sh make_img
./build.sh flash
./build.sh rttlog
```

配置选择 Scheduler=None、nanosig、nsbtn 和 lwbtn click 支持，不链接 FreeRTOS/eventhub-os。
nanosig 包根据 scheduler 选择上游 baremetal 后端，不使用 broker 或后台线程。

SysTick 每 1 ms 更新一个 64 位微秒计数，读取时短暂屏蔽中断以避免撕裂；
时钟单位为微秒、分辨率为 1 ms，覆盖业务的 10 ms 按键扫描和闪烁定时器。
主循环调用 ns_loop_poll() 派发 timer/signal，然后 WFI 等待下一次中断。
SysTick ISR 不调用 nanosig、nsbtn 或业务回调。不要在主循环长期关闭中断，
否则 SysTick 的多个到期会合并，时钟会少计；调试暂停期间也不保证墙钟时间。
主栈显式配置为 4 KiB，与原 FreeRTOS demo 默认主任务的栈预算一致，
避免直接沿用链接脚本中仅供启动代码使用的 1 KiB 默认值。

所有初始化失败路径按获取资源的逆序清理。日志通过 platform_stdout_write 转到 SEGGER RTT 通道 0。
硬件验收按上表依次测试，并检查 RTT 中的 key/led 日志。

本地构建回归：

```sh
python3 tool/test/check_freertos_config.py
python3 tool/test/check_kconfig_hygiene.py
python3 tool/test/check_build_arm.py --only mcxn-baremetal,mcxn-freertos --verbose
```

裸机 ELF 会检查至少 4 KiB 的主栈、强 SysTick_Handler、ns_loop_poll 和时钟注入入口，且不得包含 FreeRTOS、
ns_loop_run/ns_loop_start、broker 或线程入口。交叉编译不能代替板上按键、灯效和 RTT 验收。
