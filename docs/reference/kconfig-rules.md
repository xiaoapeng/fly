# Kconfig 规则（硬性）

适用范围包括 SDK 的 `arch/**/Kconfig`、`package/**/Kconfig` 和独立应用的 `Kconfig`。

## R1 choice 成员禁止被 `select` / `imply`

choice 成员的取值由 choice 自身决定，不由 `select` 决定：

```python
# kconfiglib Symbol._calc_value
elif vis == 2:
    val = 2 if self.choice.selection is self else 0   # 只读 choice.selection，从不读 rev_dep
```

`select` 只是往目标符号的 `rev_dep` 里加一项；对 choice 成员这条分支不读 `rev_dep`，所以赋值被静默丢弃，只留一条警告：

```
the choice symbol X is selected by the following symbols, but select/imply has no effect on choice symbols
```

- 禁止：`select PACKAGE_SCHEDULER_EVENTHUB_OS`（及任何 `select` 打在 choice 成员上）
- 检测：`./build.sh loadconfig ...` / `menuconfig` 输出**不得**出现 `no effect on choice symbols`
- 设计原因：choice 的不变量是「恰好一个成员为 y」；若 `select` 有效，两个项目符号同时被选中就会把两个成员拉成 y

## R2 跨模块表达选择：请求位 + 条件 default

包定义**无 prompt 的内部 bool**（请求位），需要它的模块 `select` 这个 bool，choice 再用条件 `default` 把请求转成支路：

```kconfig
# 包侧：请求位，menuconfig 不可见
config PACKAGE_SCHEDULER_REQUIRE_FREERTOS
    bool

choice PACKAGE_SCHEDULER
    prompt "Concurrency / scheduler model"
    default PACKAGE_SCHEDULER_FREERTOS if PACKAGE_SCHEDULER_REQUIRE_FREERTOS
    default PACKAGE_SCHEDULER_EVENTHUB_OS if PACKAGE_SCHEDULER_REQUIRE_EVENTHUB_OS
    default PACKAGE_SCHEDULER_NONE
```

```kconfig
# 工程侧：select 打在普通 bool 上，生效
config APP_USE_FREERTOS
    bool "Use FreeRTOS in this application"
    select PACKAGE_SCHEDULER_REQUIRE_FREERTOS
```

## R3 依赖方向：module → package，绝不反向

包 Kconfig **不得**引用 `PROJECT_*` 或任何板级/具体工程符号。否则新增一个工程就要去改包，选择器退化成项目清单。

正确方向：

```
应用 Kconfig   --select-->   package/<p>/Kconfig (普通 bool 请求位)
```

`select` 的正确用途就是「我确定需要这个普通符号」，不要用它去动别人的 choice。

## R4 choice 内多个 `default` 的顺序

- 按**声明顺序**取第一个条件成立的 default（`Choice._selection_from_defaults`）
- **无条件回退（如 `default PACKAGE_SCHEDULER_NONE`）必须放最后一行**，否则上面的条件 default 永远不生效
- 目标成员必须 visible（其 `depends on` 满足）才可能被选中；不可见时自动落到下一个 default

## R5 优先级：defconfig pin > 条件 default

成员取值的第一来源是 `user_selection`（`.config` / `menuconfig` 里的赋值），条件 default 只在其后。

- 生成的应用可在自己的 defconfig 明确指定运行方式。应用 Kconfig 有请求位时，对应的 default 会为未显式选择的配置提供默认值。
- `./build.sh saveconfig` 会把解析后的 `CONFIG_PACKAGE_SCHEDULER_*=y` 写回 defconfig，这是冗余但无害的
- 矛盾 pin **不会**报错：dual-pin 被归一为声明靠后的成员；「工程要求 A、defconfig 显式 pin B」会在下游 CMake 阶段炸（例如 freertos-demo + eventhub pin 找不到 freertos target）。不要写矛盾 pin

## R6 新增应用配置

在应用目录加载 `defconfig` 并检查最终生效的调度器。SDK 包改动还需要执行 `python3 tool/test/check_standalone_projects.py --matrix config`。

```sh
./build.sh loadconfig defconfig
grep '^CONFIG_PACKAGE_SCHEDULER_.*=y$' build/config/.config
./build.sh build
```

## R7 改 Kconfig 时沿用原块既有字段顺序

同一个 config 块内，仓库既有顺序是：

```
config SYMBOL
    bool "prompt"        # 或 string/int/choice prompt
    depends on ...       # 依赖紧跟类型
    help
        <说明>
    select ... / imply . # 全部放块尾，多条 select 紧挨着
```

- 新增 `select` / `depends on` / `default` 时**插到该字段本来该在的位置**，不要塞在 `help` 前面
- 新块的字段顺序与相邻块逐项对齐；同一文件里不要混用两种写法
- 改之前先看两三个同类块，照抄它们的顺序和缩进，不要凭个人习惯排版
- 反面例子：把 `select PACKAGE_SCHEDULER_REQUIRE_*` 插在 `bool` 与 `help` 之间（2026-10 已按本条修正为块尾、紧邻 `select PACKAGE_SEGGER_RTT` 之前）

## 变更历史

- 2026-10：11 处 `project/*/Kconfig` 的 `select PACKAGE_SCHEDULER_EVENTHUB_OS` 被确认为死代码（R1）并删除，改为 `select PACKAGE_SCHEDULER_REQUIRE_*` + `package/scheduler/Kconfig` 条件 default（R2）。同批修掉了「包 Kconfig 枚举 `PROJECT_*`」的错误做法（R3）。

## 相关文档

- [RTOS 选择语义](rtos-choice.md)
- [OS 包并入 scheduler/ 的迁移方案](../plans/scheduler-os-merge.md)
