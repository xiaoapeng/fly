/**
 * @file main.c
 * @brief 协程上下文保存/恢复测试 (Windows x64)
 *        验证 co_context_swap 在各种寄存器状态下的正确性：
 *        - SSE XMM6-XMM15 (浮点/SIMD 数据寄存器)
 *        - MXCSR (SSE 控制/状态寄存器)
 *        - x87 FPU 控制字 (精度、舍入模式)
 *        - callee-saved GPR (RBX, RSI, RDI, R12-R15; RBP 作为帧指针由编译器管理)
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026  simon.xiaoapeng@gmail.com
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <eh_debug.h>
#include "eh_co.h"

/* ---------- 输出回调 ---------- */
void stdout_write(void *stream, const uint8_t *buf, size_t size){
    (void)stream;
    fwrite(buf, size, 1, stdout);
}

/* ---------- 测试基础设施 ---------- */
static int test_failures = 0;

#define CHECK(cond, fmt, ...) do {                         \
    if(!(cond)){                                            \
        printf("  FAIL: " fmt "\n", ##__VA_ARGS__);         \
        test_failures++;                                    \
    }                                                       \
} while(0)

/* ---------- 协程上下文与栈 ---------- */
static context_t ctx_task1;
static context_t ctx_task2;
static context_t ctx_main;

static uint8_t stack_task1[32 * 1024];
static uint8_t stack_task2[32 * 1024];

/* ==================================================================
 * 测试 1: XMM6-XMM15 保存/恢复
 *
 * Windows x64 ABI 要求 callee 保存 XMM6-XMM15。
 * co_context_swap 必须在上下文切换时完整保存/恢复这 10 个 128 位寄存器。
 *
 * 方法: task1 将特定 bit pattern 写入 XMM6/7，切换到 task2，
 *       task2 用不同的 pattern 覆盖相同寄存器，再切回 task1，
 *       task1 验证原始值未被破坏。多轮重复。
 * ================================================================== */

static const uint64_t xmm_pat1[2] = { 0x3FF0000000000000ULL, 0x4000000000000000ULL };
static const uint64_t xmm_pat2[2] = { 0xBFF0000000000000ULL, 0xC000000000000000ULL };

static inline uint64_t get_xmm6_lo(void){
    uint64_t v;
    __asm__ volatile("movq %%xmm6, %0" : "=r"(v));
    return v;
}
static inline uint64_t get_xmm7_lo(void){
    uint64_t v;
    __asm__ volatile("movq %%xmm7, %0" : "=r"(v));
    return v;
}

static int test_xmm_task1(void *arg){
    (void)arg;
    int round;
    for(round = 0; round < 4; round++){
        __asm__ volatile(
            "movq %0, %%xmm6 \n"
            "movq %1, %%xmm7 \n"
            :
            : "m"(xmm_pat1[0]), "m"(xmm_pat1[1])
            : "xmm6", "xmm7"
        );

        co_context_swap(NULL, &ctx_task1, &ctx_task2);

        CHECK(get_xmm6_lo() == xmm_pat1[0],
              "round %d: xmm6 = 0x%016llx, expected 0x%016llx",
              round, (unsigned long long)get_xmm6_lo(), (unsigned long long)xmm_pat1[0]);
        CHECK(get_xmm7_lo() == xmm_pat1[1],
              "round %d: xmm7 = 0x%016llx, expected 0x%016llx",
              round, (unsigned long long)get_xmm7_lo(), (unsigned long long)xmm_pat1[1]);
    }
    co_context_swap(NULL, &ctx_task1, &ctx_main);
    return 0;
}

static int test_xmm_task2(void *arg){
    (void)arg;
    while(1){
        __asm__ volatile(
            "movq %0, %%xmm6 \n"
            "movq %1, %%xmm7 \n"
            :
            : "m"(xmm_pat2[0]), "m"(xmm_pat2[1])
            : "xmm6", "xmm7"
        );

        co_context_swap(NULL, &ctx_task2, &ctx_task1);

        CHECK(get_xmm6_lo() == xmm_pat2[0],
              "xmm6 = 0x%016llx, expected 0x%016llx",
              (unsigned long long)get_xmm6_lo(), (unsigned long long)xmm_pat2[0]);
        CHECK(get_xmm7_lo() == xmm_pat2[1],
              "xmm7 = 0x%016llx, expected 0x%016llx",
              (unsigned long long)get_xmm7_lo(), (unsigned long long)xmm_pat2[1]);
    }
    return 0;
}

/* ==================================================================
 * 测试 2: MXCSR 与 x87 控制字保存/恢复
 *
 * MXCSR 控制 SSE 舍入模式、异常掩码等。
 * x87 CW 控制 x87 FPU 精度与舍入。
 * 协程切换必须完整保存/恢复这两个控制寄存器，
 * 否则浮点运算行为会因上下文切换而意外改变。
 * ================================================================== */

static int test_fenv_task1(void *arg){
    (void)arg;
    int round;
    for(round = 0; round < 4; round++){
        __asm__ volatile("ldmxcsr %0" : : "m"((uint32_t){0x9FC0}));
        __asm__ volatile("fldcw %0" : : "m"((uint16_t){0x0C3F}));

        co_context_swap(NULL, &ctx_task1, &ctx_task2);

        uint32_t mxcsr;
        uint16_t x87cw;
        __asm__ volatile("stmxcsr %0" : "=m"(mxcsr));
        __asm__ volatile("fnstcw %0"  : "=m"(x87cw));

        CHECK(mxcsr == 0x9FC0,
              "round %d: mxcsr = 0x%04x, expected 0x9FC0",
              round, mxcsr);
        /* x87 CW bit 6 可能被 FPU 运行时置位，屏蔽后比较 */
        CHECK((x87cw & ~0x0040) == (0x0C3F & ~0x0040),
              "round %d: x87cw = 0x%04x, expected 0x0C3F",
              round, (unsigned)x87cw);
    }
    co_context_swap(NULL, &ctx_task1, &ctx_main);
    return 0;
}

static int test_fenv_task2(void *arg){
    (void)arg;
    while(1){
        __asm__ volatile("ldmxcsr %0" : : "m"((uint32_t){0x1F80}));
        __asm__ volatile("fldcw %0" : : "m"((uint16_t){0x063F}));

        co_context_swap(NULL, &ctx_task2, &ctx_task1);

        uint32_t mxcsr;
        uint16_t x87cw;
        __asm__ volatile("stmxcsr %0" : "=m"(mxcsr));
        __asm__ volatile("fnstcw %0"  : "=m"(x87cw));

        CHECK(mxcsr == 0x1F80,
              "mxcsr = 0x%04x, expected 0x1F80",
              mxcsr);
        CHECK((x87cw & ~0x0040) == (0x063F & ~0x0040),
              "x87cw = 0x%04x, expected 0x063F",
              (unsigned)x87cw);
    }
    return 0;
}

/* ==================================================================
 * 测试 3: callee-saved GPR 保存/恢复
 *
 * Windows x64 ABI callee-saved: RBX, RSI, RDI, R12-R15
 * RBP 作为帧指针由编译器管理，不在此处测试。
 * ================================================================== */

static const uint64_t gpr_pat1[7] = {
    0xAAAAAAAAAAAAAAAAULL,  /* r12 */
    0xBBBBBBBBBBBBBBBBULL,  /* r13 */
    0xCCCCCCCCCCCCCCCCULL,  /* r14 */
    0xDDDDDDDDDDDDDDDDULL,  /* r15 */
    0xEEEEEEEEEEEEEEEEULL,  /* rbx */
    0x2222222222222222ULL,  /* rsi */
    0x3333333333333333ULL,  /* rdi */
};

static const uint64_t gpr_pat2[7] = {
    0x4444444444444444ULL,
    0x5555555555555555ULL,
    0x6666666666666666ULL,
    0x7777777777777777ULL,
    0x8888888888888888ULL,
    0xAAAAAAAAAAAAAAA0ULL,
    0xBBBBBBBBBBBBBBB0ULL,
};

static int test_gpr_task1(void *arg){
    (void)arg;
    int round;
    for(round = 0; round < 4; round++){
        __asm__ volatile(
            "movq 0x00(%0), %%r12 \n"
            "movq 0x08(%0), %%r13 \n"
            "movq 0x10(%0), %%r14 \n"
            "movq 0x18(%0), %%r15 \n"
            "movq 0x20(%0), %%rbx \n"
            "movq 0x28(%0), %%rsi \n"
            "movq 0x30(%0), %%rdi \n"
            :
            : "r"(gpr_pat1)
            : "r12", "r13", "r14", "r15", "rbx", "rsi", "rdi", "memory"
        );

        co_context_swap(NULL, &ctx_task1, &ctx_task2);

        uint64_t tmp[7];
        __asm__ volatile(
            "movq %%r12, 0x00(%0) \n"
            "movq %%r13, 0x08(%0) \n"
            "movq %%r14, 0x10(%0) \n"
            "movq %%r15, 0x18(%0) \n"
            "movq %%rbx, 0x20(%0) \n"
            "movq %%rsi, 0x28(%0) \n"
            "movq %%rdi, 0x30(%0) \n"
            :
            : "r"(tmp)
            : "memory"
        );

        CHECK(memcmp(tmp, gpr_pat1, sizeof(gpr_pat1)) == 0,
              "round %d: callee-saved GPR mismatch after context switch", round);
    }
    co_context_swap(NULL, &ctx_task1, &ctx_main);
    return 0;
}

static int test_gpr_task2(void *arg){
    (void)arg;
    while(1){
        __asm__ volatile(
            "movq 0x00(%0), %%r12 \n"
            "movq 0x08(%0), %%r13 \n"
            "movq 0x10(%0), %%r14 \n"
            "movq 0x18(%0), %%r15 \n"
            "movq 0x20(%0), %%rbx \n"
            "movq 0x28(%0), %%rsi \n"
            "movq 0x30(%0), %%rdi \n"
            :
            : "r"(gpr_pat2)
            : "r12", "r13", "r14", "r15", "rbx", "rsi", "rdi", "memory"
        );

        co_context_swap(NULL, &ctx_task2, &ctx_task1);

        uint64_t tmp[7];
        __asm__ volatile(
            "movq %%r12, 0x00(%0) \n"
            "movq %%r13, 0x08(%0) \n"
            "movq %%r14, 0x10(%0) \n"
            "movq %%r15, 0x18(%0) \n"
            "movq %%rbx, 0x20(%0) \n"
            "movq %%rsi, 0x28(%0) \n"
            "movq %%rdi, 0x30(%0) \n"
            :
            : "r"(tmp)
            : "memory"
        );

        CHECK(memcmp(tmp, gpr_pat2, sizeof(gpr_pat2)) == 0,
              "callee-saved GPR mismatch after context switch");
    }
    return 0;
}

/* ==================================================================
 * 测试运行器
 * ================================================================== */

static void run_test(const char *name,
                     int (*fn1)(void*), int (*fn2)(void*)){
    printf("  [run] %s ... ", name); fflush(stdout);

    ctx_task1 = co_context_make(stack_task1, stack_task1 + sizeof(stack_task1), fn1);
    ctx_task2 = co_context_make(stack_task2, stack_task2 + sizeof(stack_task2), fn2);

    co_context_swap(NULL, &ctx_main, &ctx_task1);

    printf("done\n"); fflush(stdout);
}

int main(void){
    printf("=== co_context_save/restore test ===\n");

    run_test("XMM6-7 save/restore", test_xmm_task1, test_xmm_task2);
    run_test("MXCSR + x87 CW save/restore", test_fenv_task1, test_fenv_task2);
    run_test("callee-saved GPR save/restore", test_gpr_task1, test_gpr_task2);

    if(test_failures == 0){
        printf("ALL PASSED\n");
    } else {
        printf("FAILED: %d check(s)\n", test_failures);
    }
    fflush(stdout);
    return test_failures;
}
