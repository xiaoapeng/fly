/**
 * @file test_ehshell_escape_test_parse.c
 * @brief 测试ehshell_escape_char_parse函数
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-16
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */


#include <string.h>

#include <eh.h>
#include <eh_module.h>
#include <eh_debug.h>

#include <ehshell.h>
#include <ehshell_internal.h>
#include <ehshell_escape_char.h>

struct test_escape_tbl{
    const char *escape;
    enum ehshell_escape_char escape_char;
};

const struct test_escape_tbl test_escape_tbl[] = {
    {"\x03", /* 0x03 是 Ctrl+C */           ESCAPE_CHAR_CTRL_C_SIGINT},
    {"\x08", /* 0x08 是 Backspace */        ESCAPE_CHAR_CTRL_BACKSPACE_0},
    {"\x09", /* 0x09 是 Tab */              ESCAPE_CHAR_CTRL_TAB},
    {"\x0A", /* 0x0A 是 LF */               ESCAPE_CHAR_CTRL_J_LF},
    {"\x0D", /* 0x0D 是 CR */               ESCAPE_CHAR_CTRL_M_CR},
    {"\x1B[A", /* 0x1B[A 是 Up Arrow */     ESCAPE_CHAR_CTRL_UP},
    {"\x1B[B", /* 0x1B[B 是 Down Arrow */   ESCAPE_CHAR_CTRL_DOWN},
    {"\x1B[C", /* 0x1B[C 是 Right Arrow */  ESCAPE_CHAR_CTRL_RIGHT},
    {"\x1B[D", /* 0x1B[D 是 Left Arrow */   ESCAPE_CHAR_CTRL_LEFT},
    {"\x1B[3~", /* 0x1B[3~ 是 Delete */     ESCAPE_CHAR_CTRL_DELETE},
    {"\x1B[1~", /* 0x1B[1~ 是 Home */       ESCAPE_CHAR_CTRL_HOME},
    {"\x1B[4~", /* 0x1B[4~ 是 End */        ESCAPE_CHAR_CTRL_END},
    {"\x1B[H", /* 0x1B[H 是 Home */         ESCAPE_CHAR_CTRL_HOME},
    {"\x1B[F", /* 0x1B[F 是 End */          ESCAPE_CHAR_CTRL_END},
    {"\x1B[G",                              ESCAPE_CHAR_NUL},
    {"\x1B[6~",                             ESCAPE_CHAR_NUL},
    {"\x1B[6G",                             ESCAPE_CHAR_NUL},
    {"\x1B[334343434344545454G",            ESCAPE_CHAR_NUL},
    {"\x1B[1111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111111111111"
     "111111111111~", /* 超限测试 */         ESCAPE_CHAR_CTRL_RESET},
    {"\x1B]33434343434454\x07",             ESCAPE_CHAR_NUL},
    {"\x1B]33434343434454\x1Bt",            ESCAPE_CHAR_CTRL_RESET},
    {"\x1B]33434343434454\x1B\\",           ESCAPE_CHAR_NUL},
    {"\x1BP33434343434454\x07",             ESCAPE_CHAR_NUL},
    {"\x1BP33434343434454\x1Bt",            ESCAPE_CHAR_CTRL_RESET},
    {"\x1BP33434343434454\x1B\\",           ESCAPE_CHAR_NUL},
    {"\x1B^33434343434454\x07",             ESCAPE_CHAR_NUL},
    {"\x1B^33434343434454\x1Bt",            ESCAPE_CHAR_CTRL_RESET},
    {"\x1B^33434343434454\x1B\\",           ESCAPE_CHAR_NUL},
    {"\x1B_33434343434454\x07",             ESCAPE_CHAR_NUL},
    {"\x1B_33434343434454\x1Bt",            ESCAPE_CHAR_CTRL_RESET},
    {"\x1B_33434343434454\x1B\\",           ESCAPE_CHAR_NUL},
    {"\x1BO3",                              ESCAPE_CHAR_CTRL_RESET},
    {"\x1BOA",                              ESCAPE_CHAR_NUL},
    {"\x1BG",                               ESCAPE_CHAR_NUL},

};



int __init test_ehshell_escape_test_parse_init(void){
    struct ehshell shell;
    enum ehshell_escape_char escape_char;
    memset(&shell, 0, sizeof(struct ehshell));
    for(size_t i = 0; i < EH_ARRAY_SIZE(test_escape_tbl); i++){
        const struct test_escape_tbl *item = &test_escape_tbl[i];
        const char *pos = item->escape;
        while(*pos){
            escape_char = ehshell_escape_char_parse(&shell, *pos);
            pos++;
            if(escape_char)
                break;
        }
        if(item->escape_char != ESCAPE_CHAR_CTRL_RESET && *pos != '\0'){
            eh_errfl("提前退出, 未解析完字符串 item:%d pos:%d", i, pos - item->escape);
            continue;
        }
        if(escape_char != item->escape_char){
            eh_errfl("item:%d 预期 %d, 实际 %d", i, item->escape_char, escape_char);
            continue;
        }
    }

    return -1;
}

eh_module_level0_export(test_ehshell_escape_test_parse_init, NULL);