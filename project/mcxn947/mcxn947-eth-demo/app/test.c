/**
 * @file test.c
 * @brief 测试代码
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-06
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh.h>
#include "eh_swab.h"
#include <eh_module.h>
#include <eh_error.h>
#include <eh_mem_pool.h>
#include <eh_debug.h>
#include <eh_sleep.h>


void mem_fill(void* mem, size_t size, char start){
    for(size_t i=0; i<size; i++){
        ((char *)mem)[i] = start++;
    }
}

int mem_alloc_test(void **test_ptr, size_t num, eh_mem_pool_t test_pool){
    for(size_t i=0; i<num; i++){
        test_ptr[i] = eh_mem_pool_alloc(test_pool);
        if(test_ptr[i] == NULL) return -1;
    }
    return 0;
}

void mem_free_test(void **test_ptr, int *free_tab, eh_mem_pool_t test_pool){
    int *free_tab_start;
    for(free_tab_start = free_tab; *free_tab_start >= 0; free_tab_start++){
        eh_mem_pool_free(test_pool, test_ptr[*free_tab_start]);
    }
}


void mem_data_fill_test(void **test_ptr, size_t num, size_t size, char *fill_data_start_val){
    char fill_data;
    for(size_t i=0;i<num;i++){
        fill_data = fill_data_start_val[i];
        for(size_t j=0;j<size;j++){
            ((char*)test_ptr[i])[j] = fill_data++;
        }
    }
}

int mem_data_fill_check(void **test_ptr, size_t num, size_t size, char *fill_data_start_val){
    char fill_data;
    for(size_t i=0;i<num;i++){
        fill_data = fill_data_start_val[i];
        for(size_t j=0;j<size;j++){
            if(((char*)test_ptr[i])[j] != fill_data++){
                return -1;
            }
        }
    }
    return 0;
}

void mem_pool_test(void){
    
#define TEST_NUM 16
#define TEST_SIZE 128
    eh_mem_pool_t test_pool;
    void *test_ptr[TEST_NUM];
    eh_infoln("pool alloc test:");

    test_pool = eh_mem_pool_create(32, TEST_SIZE, TEST_NUM);
    EH_DBG_ERROR_EXEC(eh_ptr_to_error(test_pool) < 0, goto error);

    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {7,0,11,12,13,3,4,9,10,5,6,8,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("1 Pass..");
    }
    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {4,9,13,3,8,10,5,6,7,0,11,12,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("2 Pass..");
    }
    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {4,9,13,3,8,10,5,6,7,0,11,12,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("3 Pass..");
    }
    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {4,9,13,3,8,10,5,6,7,0,11,12,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("4 Pass..");
    }

    
    
    eh_mem_pool_destroy(test_pool);

    eh_infoln("Pass..");
    return ;
test_error:
    eh_mem_pool_destroy(test_pool);
error:
    eh_errln("Fail..");
}

#define _none_val(x) (x)


static int task_task(void *arg){
    (void) arg;
    // mem_pool_test();
    struct test{
        char test_data[16];
    };
    struct test td0 = {
        .test_data = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'}
    };

    struct test td1 = eh_complex_swab(td0);

    eh_debugln("test eh_swab16 %0#hx", eh_hton((short)0x1234));
    eh_debugln("test eh_swab32 %0#x", eh_hton((int)0x12345678));
    eh_debugln("test eh_swab64 %0#llx", eh_hton(0x1234567887654321ULL));
    eh_debugln("td0 |%.*hhq|", sizeof(td0),&td0);
    eh_debugln("td1 |%.*hhq|", sizeof(td1),&td1);

    int64_t t1 = 0xFFFFFFFFF;
    int32_t t2 = (int32_t)t1;
    eh_debugln("t11 %lld t2 %d", t1, t2);
    
    return 0;
}


static int __init test_init(void){
    eh_infoln("test init");
    eh_task_create("test", EH_TASK_FLAGS_DETACH, 1024, NULL, task_task);
    return 0;
}

eh_module_level9_export(test_init, NULL);
