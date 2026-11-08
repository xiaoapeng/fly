# FreeRTOS queue-set 后端设计（nanosig watcher）

> 来源：`.omc/plans/queueset-watcher-v1.md`（v5）+ `.omc/specs/deep-interview-queueset-watcher.md`。
> Pin：nanosig `b3e4d87ff777574baf00dc1199e831daa2f7c5a2`（FreeRTOS queue-set 后端已由上游 `79729fc` 提供；FLY 不再打补丁）。

## 1. 结论

FLY 的 FreeRTOS 后端把 `ns_platform_waitset` 从 **event-group bit** **整体替换**为 **`QueueSetHandle_t` + 调用方提供的成员原语**，不保留双后端并存分支。内部 `ns_platform_event_t`（broker 自唤醒 / 用户事件）实现为一个 **binary sem**。

替换理由：event-group 后端下 `waitset_add` 会把 watcher 的 `primitive.event_bit` 覆写为 bit 下标，且唯一触发路径是 `ns_platform_event_signal`→`xEventGroupSetBits`，用户无法触发 watcher（功能性残废，N1）。queue set 的成员可由调用方 / ISR 直接 give，watcher 才真正可用，且无 event-group 的 24 位硬限。

## 2. 契约（必须遵守）

### 2.1 成员两类契约

| 成员类型 | `consume_fn` | 语义 |
|---|---|---|
| readiness（binary sem） | **必须非 NULL**，并在其内 `xSemaphoreTake(member, 0)` | 否则只触发一次：成员保持满，后续 give 不再产生新 token |
| payload（queue / counting sem） | **必须非 NULL**，负责 receive / take | `consume_fn==NULL` 且成员需 drain → 明确拒绝或文档化 one-shot 语义 |

- watcher 对成员具体类型**不做假设**；binary / counting sem、queue 皆可，只要 queue set 支持。
- `waitset_add` 对 `events` 含 `IN` 之外的位（`OUT`/`ERR`）返回 `NS_E_INVAL`；RTOS 后端仅支持 `IN`。
- **notify 与 consume 是同一操作**：`select`=消费 set token；port 只报告成员，成员 drain 责任在 `consume_fn`；契约写死。
- `waitset_wait` 达 `max_completions` 即停止消费，剩余 token 留到下一轮。

### 2.2 select 与 take 成对

queue set 的 token 只在 `xQueueSelectFromSet`（内部即 `xQueueReceive`）被取走；**take 成员本身不回收 token**。因此 **select 与 take 必须成对使用**，否则 token 计数失真、可能提前触顶。

### 2.3 member 空前置

`waitset_add` 前成员必须为空：`xQueueAddToSet` 带 token 会 `pdFAIL`。port 先用 `uxQueueMessagesWaiting(member)!=0` 预检 → 返回 `NS_E_INVAL`；否则 `pdFAIL` → `NS_E_EXISTS`（两种失败**显式上抛**，不静默）。

`waitset_remove` 对"成员非空"返回 `NS_E_BUSY`（平台中立重试信号），broker 据此用 `consume_fn` 重试 drain；broker 内不出现任何 FreeRTOS 符号。

## 3. 容量与停机

queue set 本身是 queue，item 是成员句柄；每次成功 send/give 塞一个 token。

```
FIXED_CAP ≥ Σ 成员容量 + max(成员容量)
```

- `Σ 成员容量` 保证常规 select/take 成对时不被占满。
- `+ max(成员容量)` 为 **stale token 余量**：`xQueueRemoveFromSet` 不清除 set 队列中已存在的 token；watcher 注销后残留 token 需被跳过。demo 全 binary sem 时即 `Σ + 1`（= N + 1）。
- **set 满时 `prvNotifyQueueSetContainer` 的 `configASSERT`（`queue.c:3321`）在 debug/release 均停机**（本仓 `configASSERT` 无条件武装于 `FreeRTOSConfig.h:104` + `vAssertCalled` 死循环），非静默丢弃；demo 验证不触顶。
- 创建 set 时给固定容量；`waitset_add` 不做成员容量之和校验，故 `FIXED_CAP` 是隐式上限，靠本文档 sizing 约束。

## 4. stale token 跳过

`xQueueRemoveFromSet` 不清理 set 队列中已存在的成员 token（`queue.c:3252-3255`）。`waitset_wait` 取到 token 后若成员**不在 `slots` 映射中**（已注销后残留），必须**跳过：不填 completion、不解引用**，避免 UAF。

## 5. ISR 模型

- **nanosig / port 永不提供任何 ISR 函数**（永久性约束，全仓无 `FromISR` API）。
- ISR 触发是调用方责任：在自己 ISR 内用原生 `xSemaphoreGiveFromISR(member, &woken)`（+ 视需要 `portYIELD_FROM_ISR`）。
- 本仓当前无 ISR 场景，属面向未来的投资；收益当前为理论。

## 6. 配置横向影响（`configUSE_QUEUE_SETS`）

- `configUSE_QUEUE_SETS=1` 对**所有** FreeRTOS 构建全局生效，非仅 freertos-demo。
- `struct QueueDefinition` 每个 queue/sem/mutex 增一个 `pxQueueSetContainer` 指针（+4 B），并引入少量代码 / 快路径分支（`queue.c:128/213/602`）。
- 使用 queue set 时额外开销：每成员一个内核对象 + set 一个 queue（`slots[FIXED_CAP]`）。
- host 无法运行 queue-set 运行时语义（FreeRTOS-only），只能上板验证；host 侧仅做构建 / sweep 回归。

## 7. heap 预算

- set 队列 + 每成员 `Queue_t` + `slots[FIXED_CAP]` 均计入 FreeRTOS heap。
- 记录影响：`project/mcxn947/freertos-demo` heap 32 KB；其他项目默认 8 KB。
- 新增/调整成员或 `FIXED_CAP` 时必须复核 heap，避免运行时 `pvPortMalloc` 失败。

## 8. nanosig 调试日志（RTT 桥接 + 等级）

- nanosig 的所有调试输出（`ns_debug` / `ns_printf`）最终汇聚到 `src/ns_formatio.c` 里一个 **weak 的 `stdout_write()`**。
- FreeRTOS 后端的 `stdout_write()` 转调应用提供的**强符号 `platform_stdout_write`**，由 `project/mcxn947/freertos-demo/app/runtime.c` 桥接到 `SEGGER_RTT_Write(0, …)`，于是 nanosig 内部日志（broker / platform / timer）直接进同一条 RTT。
- **全局等级 Kconfig**：`CONFIG_PACKAGE_NANOSIG_LOG_LEVEL`（choice：suppress/err/warning/sys/info/debug，默认 `sys`）→ `package/nanosig/CMakeLists.txt` 映射为 `-DNS_CONFIG_DEFAULT_DEBUG_LEVEL=<0..5>`，播种 `ns_debug.c` 的 `dbg_level` 默认值。运行时仍可用 `ns_dbg_set_level()` 覆盖。
- **per-module 等级**：`ns_mdebugln(MOD, …)` 等由编译期的 `NS_DBG_MODULE_LEVEL_<MOD>` 门控。核心及平台五个模块的等级均可覆盖，例如 `target_compile_definitions(nanosig PRIVATE NS_DBG_MODULE_LEVEL_BROKER=NS_DBG_DEBUG)`；未指定时跟随 `NS_DBG_MODULE_LEVEL_DEFAULT`（默认 SYS）。公共默认等级如需覆盖，应以 PUBLIC 编译定义同时传给库及应用。

## 9. 升级到 `b3e4d87`

- 上游将 `NS_DEFINE_SLOT` 更名为 `NS_SLOT_FN_TYPEDEF`；nsbtn 同步改用新宏，保留 `nsbtn_event_slot` 类型名和回调签名。
- MPSC 修复计数器回绕竞争并移除内部 `reserve_pos` 字段，库及调用方必须一起干净重编译，不能混用旧对象文件。
- 当前 FreeRTOS 直接源码集成未启用 `NS_MPSC_CACHELINE_ALIGNED`，无需加入宿主缓存行探针或额外布局宏；源码清单、broker 栈和日志配置沿用现有接入方式。
- emit 在队列满时返回错误，不等待消费者腾空；生产者争用期间仍可能自旋。

## 10. 相关文档

- FreeRTOS 移植设计：`docs/design/freertos-port.md`
- RTOS 选择快速参考：`docs/reference/rtos-choice.md`
- 上游实现方案（v5）：`.omc/plans/queueset-watcher-v1.md`
