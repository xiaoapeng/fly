# FreeRTOS queue-set 后端设计（nanosig watcher）

FLY 在 FreeRTOS 配置下直接编译 nanosig 上游的 queue-set 后端。本文按 [SDK 默认固定的 nanosig 版本](../../package/nanosig/Kconfig) 核对，当前为 `3110cf3e3d529097a531ee259eb84d7fa4ed98f8`。应用通过 watcher 监听 FreeRTOS 队列或信号量。broker 的内部唤醒也使用一个二值信号量。 [后端源码](https://github.com/xiaoapeng/nanosig/blob/3110cf3e3d529097a531ee259eb84d7fa4ed98f8/platform/freertos/port.c)

## 注册与处理消息

1. 将调用方持有的 `ns_watcher_t` 零初始化。用 `ns_watcher_init()` 传入队列、二值信号量或计数信号量的原生句柄，监听 `NS_WAITABLE_EVENT_IN` 并提供 `handler`。FreeRTOS 后端忽略 `edge_triggered`。
2. 在 `ns_init()` 之后用 `ns_broker_add()` 注册。成员加入时必须为空，因此先注册，再启动发送方。事件掩码不受支持、句柄无效、成员非空或 FreeRTOS 拒绝加入时，后端返回 `NS_E_INVAL`。同一 waitable 已注册时返回 `NS_E_EXISTS`。再次注册同一 watcher 前，先完成注销。
3. broker 任务在成员就绪时调用 `handler`。queue set 的通知只表示成员就绪，队列数据由回调用非阻塞的 receive 或 take 取走。同一批等待结果可能合并同一成员的多条通知，因此每次回调都应把当前成员处理到空。
4. 回调应尽快返回。需要在应用 loop 里做的工作可由回调发出应用自己的 nanosig 信号。把 `ns_broker_add()`、`ns_broker_remove()` 和 `ns_shutdown()` 留在应用任务里调用。在回调里调用这些函数会让 broker 等待自身。

生产方按所在上下文使用 FreeRTOS 的发送接口。ISR 可用 `xQueueSendFromISR()` 向队列发消息，或用 `xSemaphoreGiveFromISR()` 释放信号量。若操作唤醒了更高优先级的任务，可用 `portYIELD_FROM_ISR()` 请求调度。 [watcher 接口](https://github.com/xiaoapeng/nanosig/blob/3110cf3e3d529097a531ee259eb84d7fa4ed98f8/include/nanosig/nanosig_broker.h#L29-L146) · [注册与等待的实现](https://github.com/xiaoapeng/nanosig/blob/3110cf3e3d529097a531ee259eb84d7fa4ed98f8/platform/freertos/port.c#L549-L694)

## 停止与注销

**注销 FreeRTOS 数据队列的 watcher 时，队列里剩下的未读消息会被丢弃。** FreeRTOS 要求成员为空才能从 queue set 摘除，当前后端先调用 `xQueueReset()`，再调用 `xQueueRemoveFromSet()`。注销返回 `NS_OK` 或 `NS_E_INVAL`。即使队列起初非空，调用方也收不到可用于重试的 `NS_E_BUSY`。 [注销接口约定](https://github.com/xiaoapeng/nanosig/blob/3110cf3e3d529097a531ee259eb84d7fa4ed98f8/include/nanosig/nanosig_port.h#L558-L575) · [FreeRTOS 实现](https://github.com/xiaoapeng/nanosig/blob/3110cf3e3d529097a531ee259eb84d7fa4ed98f8/platform/freertos/port.c#L604-L650)

退出时按下面的顺序处理。

1. 停止向该队列发送消息的任务和中断来源。
2. 让仍处于注册状态的 `handler` 处理剩余消息，在应用任务确认处理完成。
3. 从应用任务调用 `ns_broker_remove()`。成功后调用 `ns_watcher_deinit()`，再释放该单元持有的队列和相关资源。如果回调把工作投递到了其他任务或 loop，释放这些工作的上下文前也要确认它们已经结束。
4. 完成各单元清理后，再关闭 nanosig 运行时。

queue set 里可能仍有已注销成员的就绪通知。后端在等待时跳过无对应注册项的通知，也会在注册新成员前清理残留通知，防止复用句柄时误派发。其他已注册成员的数据保留在各自的队列内。 [残留通知处理](https://github.com/xiaoapeng/nanosig/blob/3110cf3e3d529097a531ee259eb84d7fa4ed98f8/platform/freertos/port.c#L193-L215)

## 容量与内存预算

`NANOSIG_FREERTOS_WAITSET_CAP` 默认是 32 个就绪通知槽。添加成员时，后端检查 **成员容量总和 + 最大成员容量** 是否超过该上限，超出时返回 `NS_E_TOO_MANY_HANDLES`。容量按队列深度或信号量最大计数计算，二值信号量占 1。给 broker 估算容量时也要计入它的内部唤醒信号量。最大成员容量这一项为残留通知预留空间。 [容量校验](https://github.com/xiaoapeng/nanosig/blob/3110cf3e3d529097a531ee259eb84d7fa4ed98f8/platform/freertos/port.c#L564-L586)

调整上限时，编译 nanosig 和使用其平台类型的目标必须使用相同的值，因为它决定了公开 waitset 类型里数组的大小。评估内存时分别考虑 FreeRTOS heap 中的队列与信号量、waitset 本身，以及清理残留通知时的临时栈空间。FLY 的 heap 大小通过 `CONFIG_PACKAGE_FREERTOS_TOTAL_HEAP_SIZE` 配置。 [FreeRTOS 包配置](../../package/scheduler/freertos/Kconfig) · [FreeRTOS 模板默认配置](../../templates/apps/freertos-nanosig/defconfig)

## FLY 模板与验证

- FLY 的 [FreeRTOS 配置](../../package/scheduler/freertos/inc/FreeRTOSConfig.h) 默认启用 queue set、动态分配和互斥量，满足 nanosig FreeRTOS 后端的编译要求。
- nanosig 模板在 [应用运行时](../../templates/apps/common/nanosig/app/runtime.c) 把日志输出接到 RTT 通道 0。FreeRTOS 使用 `platform_stdout_write()`。默认日志级别由 [nanosig 的 Kconfig](../../package/nanosig/Kconfig) 控制，也可在运行时用 `ns_dbg_set_level()` 调整。单模块的编译期日志等级用 `NS_DBG_MODULE_LEVEL_<MOD>` 设置。
- [FLY 回归](../CI.md) 验证从模板生成、编译和打包固件。模板的 [心跳业务](../../templates/apps/common/nanosig/app/biz/heartbeat.c) 演示周期定时器。应用接入自定义 watcher 时，应单独验证队列满载、停止生产后排空和注销。目标板上的 ISR 触发和时序需要在相应板卡上验证。

## 迁移背景

早期后端用 event-group bit 表示事件。改用 FreeRTOS queue set 后，watcher 可以等待应用提供的原生队列或信号量。FLY 在历史版本 `b3e4d87` 集成时也同步适配了上游 slot 宏的改名和 MPSC 内部布局变更。早期板卡与构建记录见 [FreeRTOS 首次移植设计](freertos-port.md)。
