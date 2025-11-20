/**
 * @file tty_io.c
 * @brief 捕获标准输入和输出
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-12
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <eh.h>
#include <eh_ringbuf.h>
#include <eh_module.h>
#include <eh_debug.h>
#include <eh_platform.h>
#include <ehshell.h>
#include <epoll_hub.h>

static struct termios s_old_in, s_new_in;
static struct termios s_old_out, s_new_out;
static ehshell_t *s_shell = NULL;
static void tty_io_epool_callback(uint32_t events, void *arg){
    (void)arg;
    uint8_t buf[128];
    ssize_t r = 0;
    eh_ringbuf_t* input_ringbuf = NULL;
    input_ringbuf = ehshell_input_ringbuf(s_shell);
    if(events & EPOLLIN){
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

static struct epoll_fd_action action = {
    .callback = tty_io_epool_callback,
    .arg = NULL,
};

const struct ehshell_config shell_config = {
    .host = "eventos",
    .input_linebuf_size = 64,
    .input_ringbuf_size = 256,
    .write = tty_write,
    .finish = tty_finish,
    .input_ringbuf_process_finish = NULL,
    .max_command_count = 10,
};

int __init tty_io_init(void){
    eh_debugfl("tty_io_init");
    // 获取当前终端属性
    tcgetattr(STDIN_FILENO, &s_old_in);
    s_new_in = s_old_in;

    // 关闭回显和熟模式（开启原始模式）
    s_new_in.c_lflag &= (tcflag_t)~(ECHO | ICANON | ISIG);

    // 立即应用设置
    tcsetattr(STDIN_FILENO, TCSANOW, &s_new_in);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        eh_merrfl(TTY_IO, "fcntl(F_GETFL) %s", strerror(errno));
        return -1;
    }

    // 设置 O_NONBLOCK
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        eh_merrfl(TTY_IO, "fcntl(F_SETFL) %s", strerror(errno));
        return -1;
    }


    // 获取当前终端属性
    tcgetattr(STDOUT_FILENO, &s_old_out);
    s_new_out = s_old_out;

    // 关闭回显和熟模式（开启原始模式）
    s_new_out.c_oflag  &= (tcflag_t)~(OPOST);

    // 立即应用设置
    tcsetattr(STDOUT_FILENO, TCSANOW, &s_new_out);
    // 注册标准输入到epoll
    epoll_hub_add_fd(STDIN_FILENO, EPOLLIN, &action);

    s_shell = ehshell_create(&shell_config);
    if(eh_ptr_to_error(s_shell) < 0){
        eh_merrfl(TTY_IO, "ehshell_create failed %d", eh_ptr_to_error(s_shell));
        return -1;
    }

    return 0;
}

void __exit tty_io_exit(void){
    // 恢复原始终端属性
    // 注销标准输入从epoll
    ehshell_destroy(s_shell);
    epoll_hub_del_fd(STDIN_FILENO);
    tcsetattr(STDOUT_FILENO, TCSANOW, &s_old_out);
    tcsetattr(STDIN_FILENO, TCSANOW, &s_old_in);
}


eh_module_level0_export(tty_io_init, tty_io_exit);
