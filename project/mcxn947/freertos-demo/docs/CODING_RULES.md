# freertos-demo 编码规则

本文件只约束 `project/mcxn947/freertos-demo` 这个 demo 工程；仓库级约定见根目录 `AGENTS.md`。

## 1. init 函数必须用 goto 做反向清理

适用于本 demo 所有 `*_init()` 函数（`runtime_init()`、`test_init()`，以及后续新增模块的 init）。

规则：

1. 按资源获取顺序初始化，每一步立即检查返回码。
2. 任一步失败就 `goto out_<该步已获取的资源>`，**不要**在每个失败分支里就地展开清理代码。
3. 清理标签集中在函数末尾，按资源获取的**逆序**排列；每个标签释放一步资源，然后 fallthrough 到下一个标签继续释放。
4. 成功路径在清理标签之前 `return NS_OK;` 提前返回，不经过清理代码。
5. 清理分支只做释放，释放返回值一律 `(void)` 忽略；`rc` 保留最初失败的原因，不被后续清理覆盖。
6. 标签命名 `out_<资源名>`；没有资源可释放的入口（如参数校验失败）直接 `goto out`，函数末尾保留一个裸 `out:` 只负责 `return rc;`。

示例 —— `app/test.c` 的 `test_init()`（timer → signal connection → start，三步逆序释放）：

```c
int test_init(void)
{
    ns_loop_t *main_loop = runtime_get();
    int rc;

    if(main_loop == NULL) {
        rc = NS_E_INVAL;
        goto out;
    }

    rc = ns_timer_init(&test_timer, TEST_TIMER_INTERVAL_US, NS_TIMER_ATTR_REPEAT);
    if(rc != NS_OK) {
        goto out;
    }

    rc = ns_signal_connect_typed(test_timer.signal, on_timeout, ns_no_payload_t,
                                 main_loop, NULL, &test_conn);
    if(rc != NS_OK) {
        goto out_timer;
    }

    rc = ns_timer_start(&test_timer);
    if(rc != NS_OK) {
        goto out_conn;
    }
    return NS_OK;

out_conn:
    (void)ns_signal_disconnect(&test_conn);
out_timer:
    (void)ns_timer_deinit(&test_timer);
out:
    return rc;
}
```

反例（不要这样写）：每个失败分支各自复制一份释放序列。加第四个资源时必然有一处漏改，且失败路径上的释放顺序依赖分支写法而不是依赖获取顺序。

这条规则与 nanosig 上游 demo（`dl/nanosig-src/demos/*.c`）的写法一致，`AGENTS.md` 里也要求多资源示例必须展示内核风格 `goto` 清理。

## 2. deinit 函数

`*_deinit()` 是成对的释放入口，允许顺序释放，但释放顺序必须是 init 的逆序，任一前置步骤失败不得跳过后面的释放。
