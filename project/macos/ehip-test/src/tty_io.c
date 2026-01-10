/**
 * @file tty_io.c
 * @brief 捕获标准输入和输出
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-12
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#ifdef __APPLE__
#include <sys/event.h>  // macOS 使用 kqueue
#include <signal.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <eh.h>
#include <eh_ringbuf.h>
#include <eh_module.h>
#include <eh_debug.h>
#include <eh_platform.h>
#include <ehshell.h>
#include <ehshell_module.h>
#include <kqueue_hub.h>

static struct termios s_old_in, s_new_in;
static struct termios s_old_out, s_new_out;
static ehshell_t *s_shell = NULL;

static void tty_io_epool_callback(int fd, int16_t filter, void *arg){
    (void)arg;
    (void)fd;
    uint8_t buf[128];
    ssize_t r = 0;
    eh_ringbuf_t* input_ringbuf = NULL;
    input_ringbuf = ehshell_input_ringbuf(s_shell);
    if(filter == EVFILT_READ){
        do{
            r = read(STDIN_FILENO, buf, sizeof(buf));
            if(r <= 0) 
                break;
            // eh_debugfl("read %d bytes |%.*hhq|", (int)r, (int)r, buf);
            eh_ringbuf_write(input_ringbuf, buf, (int32_t)r);
            ehshell_notify_processor(s_shell);
        }while(1);
    }
}

static void tty_write(ehshell_t* ehshell, const char *buf, size_t len){
    (void)ehshell;
    fwrite(buf, 1, len, stdout);
}

static void tty_finish(ehshell_t* ehshell){
    (void)ehshell;
    fflush(stdout);
}

#ifdef __APPLE__
// macOS 特殊处理：需要恢复 SIGTTIN/SIGTTOU 信号处理
static struct sigaction s_old_ttin_sa, s_old_ttou_sa;
#endif

const struct ehshell_config shell_config = {
    .host = "eventos",
    .input_linebuf_size = 64,
    .input_ringbuf_size = 256,
    .stream_write = tty_write,
    .stream_finish = tty_finish,
    .input_ringbuf_process_finish = NULL,
    .max_command_count = 10,
};


// 注册标准输入到epoll
const struct kqueue_event_action tty_read_action = {
    .callback = tty_io_epool_callback,
    .arg = NULL,
};

int __init tty_io_init(void){
    eh_debugfl("tty_io_init");

    // macOS 特殊处理：忽略 SIGTTIN/SIGTTOU 信号
    // 防止后台进程尝试读取终端时被挂起
    struct sigaction sa_ignore = {0};
    sa_ignore.sa_handler = SIG_IGN;
    
    if (sigaction(SIGTTIN, &sa_ignore, &s_old_ttin_sa) == -1) {
        eh_merrfl(TTY_IO, "sigaction SIGTTIN failed: %s", strerror(errno));
    }
    
    if (sigaction(SIGTTOU, &sa_ignore, &s_old_ttou_sa) == -1) {
        eh_merrfl(TTY_IO, "sigaction SIGTTOU failed: %s", strerror(errno));
    }
    
    // 获取当前终端属性
    if (tcgetattr(STDIN_FILENO, &s_old_in) == -1) {
        eh_merrfl(TTY_IO, "tcgetattr stdin failed: %s", strerror(errno));
        return -1;
    }
    s_new_in = s_old_in;

    // 关闭回显和熟模式（开启原始模式）
    s_new_in.c_lflag &= (tcflag_t)~(ECHO | ICANON | ISIG | IEXTEN);
    
    // 设置最小字符数和超时
    s_new_in.c_cc[VMIN] = 1;  // 至少读取1个字符
    s_new_in.c_cc[VTIME] = 0; // 无超时

    // 关闭输入处理
    s_new_in.c_iflag &= (tcflag_t)~(IXON | IXOFF | ICRNL | INLCR | IGNCR);
    
    // 关闭输出处理
    s_new_in.c_oflag &= (tcflag_t)~(OPOST);

    // 立即应用设置
    if (tcsetattr(STDIN_FILENO, TCSANOW, &s_new_in) == -1) {
        eh_merrfl(TTY_IO, "tcsetattr stdin failed: %s", strerror(errno));
        return -1;
    }

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        eh_merrfl(TTY_IO, "fcntl(F_GETFL) %s", strerror(errno));
        return -1;
    }

    // 设置 O_NONBLOCK
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        eh_merrfl(TTY_IO, "fcntl(F_SETFL) %s", strerror(errno));
        return -1;
    }

    // 获取当前终端属性
    if (tcgetattr(STDOUT_FILENO, &s_old_out) == -1) {
        eh_merrfl(TTY_IO, "tcgetattr stdout failed: %s", strerror(errno));
        return -1;
    }
    s_new_out = s_old_out;

    // 关闭输出处理
    s_new_out.c_oflag &= (tcflag_t)~(OPOST);

    // 立即应用设置
    if (tcsetattr(STDOUT_FILENO, TCSANOW, &s_new_out) == -1) {
        eh_merrfl(TTY_IO, "tcsetattr stdout failed: %s", strerror(errno));
        return -1;
    }
    

    if(kqueue_hub_add_fd(STDIN_FILENO, EVFILT_READ, EV_ENABLE, &tty_read_action) < 0){
        eh_merrfl(TTY_IO, "kqueue_hub_add_fd failed");
        return -1;
    }

    s_shell = ehshell_create(&shell_config);
    if(eh_ptr_to_error(s_shell) < 0){
        eh_merrfl(TTY_IO, "ehshell_create failed %d", eh_ptr_to_error(s_shell));
        return -1;
    }

    return 0;
}

void __exit tty_io_exit(void){
    // 恢复原始终端属性
    ehshell_destroy(s_shell);
    kqueue_hub_del_fd(STDIN_FILENO, EVFILT_READ);
    
    // 恢复终端属性
    tcsetattr(STDOUT_FILENO, TCSANOW, &s_old_out);
    tcsetattr(STDIN_FILENO, TCSANOW, &s_old_in);

    // macOS: 恢复信号处理
    sigaction(SIGTTIN, &s_old_ttin_sa, NULL);
    sigaction(SIGTTOU, &s_old_ttou_sa, NULL);
}

ehshell_module_default_shell_export(tty_io_init, tty_io_exit);