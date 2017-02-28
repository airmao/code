/**
 * @file echo_client.c
 * @brief client
 * @note 
 * @version 0.1
 */
/**/
#include <netdb.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "echo.h"

static void *echo_client_thread1_proc(void *arg) 
{
    int                fd;
    int                rv;
    struct sockaddr_un addr;
    char               in_string[BUFLEN] = "Hello World";
    int                len;
    echo_info_t        echo_info;
    char               out_string[BUFLEN] = "";

    printf("start thread1 :\n");

    memset(&addr, 0, sizeof(addr));
    addr.sun_family  = AF_UNIX;
    strcpy(addr.sun_path, "echo_server");
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0); 
    if (fd <= 0) {
        printf("fd fail %d\n", fd);
        return (void *)0;
    }

    rv = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    if (rv != 0) {
        printf("con fail %d\n", rv);
        return (void *)0;
    }

    memset(&echo_info, 0, sizeof(echo_info_t));
    echo_info.cvt_type = ECHO_TYPE_UNSET;
    memcpy(echo_info.buf, in_string, BUFLEN);
    
    len = send(fd, &echo_info, sizeof(echo_info_t), 0);
    if (len < 0) {
        printf("send fail %d\n", len);
        return (void *)0;
    }

    len = recv(fd, out_string, BUFLEN, 0);
    if (len <= 0) {
        printf("recv failed rv %d\n", len);
        return (void *)0;
    }
    
    printf("thread1 ECHO_TYPE_UNSET string %s to %s\n", in_string, out_string);
    return (void *)0;
}

static void *echo_client_thread2_proc(void *arg) 
{
    int                fd;
    int                rv;
    struct sockaddr_un addr;
    char               in_string[BUFLEN] = "Hello World";
    int                len;
    echo_info_t        echo_info;
    char               out_string[BUFLEN] = "";

    printf("start thread2 :\n");

    memset(&addr, 0, sizeof(addr));
    addr.sun_family  = AF_UNIX;
    strcpy(addr.sun_path, "echo_server");
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0); 
    if (fd <= 0) {
        printf("fd fail %d\n", fd);
        return (void *)0;
    }

    rv = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    if (rv != 0) {
        printf("con fail %d\n", rv);
        return (void *)0;
    }

    memset(&echo_info, 0, sizeof(echo_info_t));
    echo_info.cvt_type = ECHO_TYPE_LTOU_ALL;
    memcpy(echo_info.buf, in_string, BUFLEN);
    
    len = send(fd, &echo_info, sizeof(echo_info_t), 0);
    if (len < 0) {
        printf("send fail %d\n", len);
        return (void *)0;
    }

    len = recv(fd, out_string, BUFLEN, 0);
    if (len <= 0) {
        printf("recv failed rv %d\n", len);
        return (void *)0;
    }
    
    printf("thread2 ECHO_TYPE_NUM string %s to %s\n", in_string, out_string);
    return (void *)0;
}

/**
 * @brief  main函数测试函数
 *
 * @argc: 未使用
 * @argv: 未使用
 *
 * @return void
 */
int main(int argc, char *argv[])
{
    int                rv;
    pthread_t          client_pthread1;
    pthread_t          client_pthread2;

    printf("start client:\n");

    rv = pthread_create(&client_pthread1, NULL, echo_client_thread1_proc, NULL);
    if (rv != 0) {
        printf("pthread_create client_pthread1 failed!\n");
        return rv;
    }

    rv = pthread_create(&client_pthread2, NULL, echo_client_thread2_proc, NULL);
    if (rv != 0) {
        printf("pthread_create client_pthread2 failed!\n");
        return rv;
    }

    for (;;)
        sleep(10);

    return 0;    
}

