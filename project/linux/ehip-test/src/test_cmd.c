/**
 * @file test_cmd.c
 * @brief 测试shell 和 命令行解析
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-19
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh.h>
#include <eh_error.h>
#include <eh_formatio.h>
#include <eh_module.h>
#include <eh_debug.h>
#include <ehshell.h>
#include <argparse.h>
static const char *const usages[] = {
    "testx [options]",
    NULL,
};

void do_test(ehshell_cmd_context_t *cmd_context, int argc, const char *argv[]){
    int force = 0;
    int bool_1 = 0;
    int bool_2 = 0;
    const char *path = NULL;
    const char *name = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_BOOLEAN('f', "force", &force, "force mode", NULL, 0, 0),
        OPT_BOOLEAN('a', "bool_1", &bool_1, "bool_1 value", NULL, 0, 0),
        OPT_BOOLEAN('b', "bool_2", &bool_2, "bool_2 value", NULL, 0, 0),
        OPT_STRING('p', "path", &path, "path value", NULL, 0, 0),
        OPT_STRING('n', "name", &name, "name value", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, ehshell_command_stream(cmd_context), usages, 0);
    argparse_describe(&argparse, "\nTest program for callback functionality.", NULL);
    argc = argparse_parse(&argparse, argc, argv);
    if(argc < 0){
        goto quit;
    }
    eh_stream_printf(ehshell_command_stream(cmd_context), "force %d path %s name %s bool_1 %d bool_2 %d\r\n", force, path, name, bool_1, bool_2);
quit:
    ehshell_command_finish(cmd_context);
}

static struct ehshell_command_info ehshell_command_info_tbl[] = {
    {
        .command = "test0",
        .description = "Test command 0.",
        .usage = "test0 -h show help",
        .flags = 0,
        .do_function = do_test,
        .do_event_function = NULL
    },{
        .command = "test1",
        .description = "Test command 1.",
        .usage = "test1 -h show help",
        .flags = 0,
        .do_function = do_test,
        .do_event_function = NULL
    }
};

int __init test_cmd_init(void){
    int ret;
    ret = ehshell_register_commands(ehshell_default(), ehshell_command_info_tbl, EH_ARRAY_SIZE(ehshell_command_info_tbl));
    if(ret < 0){
        eh_errfl("ehshell_register_commands failed, ret = %d", ret);
        return ret;
    }
    return 0;
}


eh_module_level1_export(test_cmd_init, NULL);
