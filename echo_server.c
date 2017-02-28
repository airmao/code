/**
 * @file echo_server.c
 * @brief server
 * @note 
 * @version 0.1
 */
/*test*/
#include <netdb.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "echo.h"

/* 数据管理结构 */
typedef struct echo_serv_data_s {                
    echo_serv_trs_type_t cvt_type;                 /* 字符串转换方式 */
    int                  fd;                       /* socket端口 */
    int                  char_cnt[ECHO_TYPE_NUM];  /* 转换字符统计 */
    sem_t                echo_sema;
} echo_serv_data_t;

echo_serv_data_t my_echo_serv;

/* echo CLI显示输出 */
static char *echo_cvt_type[] = {
    "None",
    "uptol",
    "ltoup"
};

static void echo_lock(void)
{
    sem_wait(&my_echo_serv.echo_sema);

    return;
}

static void echo_unlock(void)
{
    sem_post(&my_echo_serv.echo_sema);

    return;
}

/**
 * @brief  200ms延迟
 *
 * @void
 *
 * @return void
 */
static void echo_serv_perchar_delay(void) 
{
    struct timespec request;
    struct timespec remain;
    
    request.tv_sec = 0;
    request.tv_nsec = ECHO_DELAY_USEC;

    nanosleep(&request, &remain);

    return;
}

/**
 * @brief  字符转换
 *
 * @str: 输入转换以及输出的字符串
 *
 * @return void
 */
void echo_serv_cvt_strings(char *str, echo_serv_trs_type_t cvt_type)
{
    int len;
    int i;

    len = strlen(str);

    for (i = 0; i < len; i++) {
        switch (cvt_type) {
        case ECHO_TYPE_LTOU_ALL:
            str[i] = toupper(str[i]);

            echo_serv_perchar_delay();
            
            break;
        case ECHO_TYPE_UTOL_ALL:
            str[i] = tolower(str[i]);

            echo_serv_perchar_delay();
            
            break;
        case ECHO_TYPE_UNSET:
        default:
            break;
        }
    }

    return;
}

/**
 * @brief  SERVER端字符串接收处理函数
 *
 * @arg: 未使用
 *
 * @return void
 */
static void *echo_serv_thread_proc(void *arg) 
{
    echo_info_t          echo_info;
    int                  rcv_len;
    int                  snd_len;
    int                  clfd;
    echo_serv_trs_type_t cvt_type;
    

    clfd = *((int *)arg);

    memset(&echo_info, 0, sizeof(echo_info_t));
    
    rcv_len = recv(clfd, &echo_info, sizeof(echo_info_t), 0);
    if (rcv_len <= 0) {
        close(clfd);
        return (void *)0;
    }

    /* proc buf */
    echo_info.buf[BUFLEN] = '\0';

    echo_lock();
    cvt_type = echo_info.cvt_type >= ECHO_TYPE_NUM ? my_echo_serv.cvt_type : echo_info.cvt_type;
    echo_unlock();
    
    echo_serv_cvt_strings(echo_info.buf, cvt_type);
       
    snd_len = send(clfd, echo_info.buf, BUFLEN, 0);

    echo_lock();
    my_echo_serv.char_cnt[my_echo_serv.cvt_type] += strlen(echo_info.buf);
    echo_unlock();

    close(clfd);

    return (void *)0;
}

static int echo_serv_lockfile(int fd) 
{
    struct flock  fl;
    
    fl.l_type   = F_WRLCK;
    fl.l_start  = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len    = 0;

    return (fcntl(fd, F_SETLK, &fl));
}

/**
 * @brief  单实例实现
 *
 * @void
 *
 * @return void
 */
static bool echo_serv_already_running(void) 
{
    int     fd;
    char    buf[BUFLEN];
    
    fd = open("../echos.pid", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR |S_IRGRP | S_IROTH);
    if (fd < 0) {
        return TRUE;
    }

    if (echo_serv_lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
        }

        return TRUE;
    }
    
    ftruncate(fd, 0);
    
    sprintf(buf, "%ld", (long) getpid());
    write(fd, buf, strlen(buf) + 1);
    
    return FALSE;
}

/**
 * @brief  server启动
 *
 * @void
 *
 * @return void
 */
static void echo_serv_cvt_start(void)
{
    int        clfd;
    pthread_t  echo_pid;
    
    while (1) {
        clfd = accept(my_echo_serv.fd, NULL, NULL);
        if (clfd < 0) {
            continue;
        }

        if (pthread_create(&echo_pid, NULL, echo_serv_thread_proc, &clfd) != 0) {
            close(clfd);
            printf("can't create thread\n");
            continue;
        }
    }

    return;
}

/**
 * @brief  socket初始化
 *
 * @void
 *
 * @return -1/0
 */
static int echo_serv_ipc_init(void) 
{
    struct sockaddr_un addr;
    int                fd;
    
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "echo_server");
    unlink("echo_server");    
        
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (bind(fd, (struct sockaddr *)(&addr), sizeof(struct sockaddr_un)) < 0) {
        close(fd);
        return -1;
    }
    

    if (listen(fd, QLEN) < 0) {
        close(fd);
        return -1;
    }

    my_echo_serv.fd = fd;

    echo_serv_cvt_start();

    return 0;
}



/**
 * @brief  判断是否支持转换类型
 *
 * @param: 转换类型
 *
 * @return TRUE/FALSE
 */
static bool echo_serv_cvt_type_valid(int param)
{
    bool param_valid;

    switch (param) {
    case ECHO_TYPE_UNSET:
    case ECHO_TYPE_UTOL_ALL:
    case ECHO_TYPE_LTOU_ALL:
        param_valid = TRUE;
        break;
    default:
        param_valid = FALSE;
    }

    return param_valid;
}


static int echo_serv_init(void)
{
    int   result;

    my_echo_serv.cvt_type = ECHO_TYPE_UNSET;

    sem_init(&my_echo_serv.echo_sema, 0, 1);

    result = echo_serv_ipc_init();
    if (result < 0) {
        printf("echo_serv_ipc_init fail %d\n", result);
        return result;
    }
    
    return 0;
}

/**
 * @brief  main函数
 *
 * @argc: 未使用
 * @argv: 未使用
 *
 * @return 0或失败
 */
int main(int argc, char *argv[]) 
{
    int   result;
    pid_t pid;
    
    daemon(1, 1);

    if (echo_serv_already_running()) {
        printf("running again \n");
        return -1;
    }
    
    while (1) {
        if ((pid = fork()) < 0) {
            printf("fork error!\n");
            exit(0);
        } else if (pid == 0) {
            printf("create child task success!\n");
            result = echo_serv_init();
            if (result < 0) {
                printf("echo_serv_init failed %d\n", result);
                exit(0);
            }

            for (;;) {
                echo_serv_perchar_delay();
            }  
            
            exit(0);      
        } else {
            printf("run parent task\n");
            
            if (waitpid(pid, NULL, 0) != pid) {
                printf("waitpid error\n");
            } else {
                printf("child task is error, restart task!\n");
            }
        }
    }

    exit(0);
}

