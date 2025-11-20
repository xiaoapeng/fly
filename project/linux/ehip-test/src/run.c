/**
 * @file run.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-03
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <eh.h>
#include <eh_event.h>
#include <eh_timer.h>
#include <eh_signal.h>
#include <eh_debug.h>
#include <eh_module.h>
#include <unistd.h>

void stdout_write(void *stream, const uint8_t *buf, size_t size){
    (void)stream;
    fwrite_unlocked(buf, size, 1, stdout);
}


EH_DEFINE_STATIC_CUSTOM_SIGNAL(timer_signal, eh_event_timer_t, EH_TIMER_INIT(timer_signal.custom_event));

// static void cleanup_unix_signal_handlers(void) {
//     // 解除SIGIO阻塞
//     sigset_t set;
//     sigemptyset(&set);
//     sigaddset(&set, SIGIO);
//     pthread_sigmask(SIG_UNBLOCK, &set, NULL);

//     // 重置所有信号处理程序为默认值
//     const int signal_s[] = {
//         SIGBUS,  SIGFPE,  SIGHUP, 
//         SIGINT,  SIGPIPE, SIGQUIT,
//         SIGSYS,  SIGTERM, SIGTRAP, SIGUSR1,
//         SIGUSR2, SIGIOT
//     };
    
//     struct sigaction default_action;
//     sigemptyset(&default_action.sa_mask);
//     default_action.sa_flags = 0;
//     default_action.sa_handler = SIG_DFL;

//     for (size_t i = 0; i < sizeof(signal_s)/sizeof(signal_s[0]); i++) {
//         sigaction(signal_s[i], &default_action, NULL);
//     }
// }


// static void terminate(int sig_no){
//     eh_infofl("terminate %d", sig_no);
//     cleanup_unix_signal_handlers();
//     eh_signal_dispatch_loop_request_quit_from_task(eh_task_main());
// }

// static void setup_unix_signal_handlers(void) {
//     sigset_t set;
//     sigemptyset(&set);
//     sigaddset(&set, SIGIO);
//     pthread_sigmask(SIG_BLOCK, &set, NULL);

//     const int signal_s[] = {
//         SIGBUS,  SIGFPE,  SIGHUP, 
//         SIGINT,  SIGPIPE, SIGQUIT,
//         SIGSYS,  SIGTERM, SIGTRAP, SIGUSR1,
//         SIGUSR2, SIGIOT 
//     };
    
//     struct sigaction sa;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = SA_RESTART; 
//     sa.sa_handler = terminate;

//     // 设置每个信号的处理程序
//     for (size_t i = 0; i < sizeof(signal_s)/sizeof(signal_s[0]); i++) {
//         if (sigaction(signal_s[i], &sa, NULL) == -1) {
//             eh_errfl("Failed to set signal handler: %s", strsignal(signal_s[i]));
//         }
//     }
// }



static void timer_slot_function(eh_event_t *e, void *slot_param){
    (void)slot_param;
    (void)e;
    // static int cnt = 0;
    // eh_infofl("timer slot function! cnt: %d", cnt++);
    // eh_stream_printf(EH_STDOUT, "TEST timer slot function! cnt: %d", cnt);
    // eh_stream_putc(EH_STDOUT, '-');
    // eh_stream_putc(EH_STDOUT, '-');
    // eh_stream_putc(EH_STDOUT, '-');
    // eh_stream_putc(EH_STDOUT, '-');
    // eh_stream_puts(EH_STDOUT, "test\r\n");
    
}
static EH_DEFINE_SLOT(timer_slot, timer_slot_function, NULL);

static int __init run_init(void){
    eh_timer_advanced_init(eh_signal_to_custom_event(&timer_signal), (eh_sclock_t)eh_msec_to_clock(1000*5), EH_TIMER_ATTR_AUTO_CIRCULATION);
    eh_signal_slot_connect(&timer_signal, &timer_slot);
    eh_timer_start(eh_signal_to_custom_event(&timer_signal));
    // setup_unix_signal_handlers();
    eh_debugfl("run init!");
    return 0;
}

static void __exit run_exit(void){
    eh_debugfl("run exit!");
    // cleanup_unix_signal_handlers();
    eh_timer_stop(eh_signal_to_custom_event(&timer_signal));
    eh_signal_slot_disconnect(&timer_signal, &timer_slot);
}

eh_module_level8_export(run_init, run_exit);
