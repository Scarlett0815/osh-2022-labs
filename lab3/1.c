#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

struct Pipe {
    int fd_send;
    int fd_recv;
};

void *handle_chat(void *data) {
    struct Pipe *pipe = (struct Pipe *)data;
    char buffer[2000];
    char tmp[1000];
    ssize_t len;
    int index = 0;
    int send_flag = 0;
    int count = 0;
    while ((len = recv(pipe->fd_send, buffer + index, 1000, 0)) > 0){
// send(pipe->fd_recv, buffer, len + 8, 0);
        //printf("%d\n",count++);
        int tmp_stop = 0;
        for (int i = 0;i < len;i ++){
            if (buffer[i] == '\n'){
                if (send_flag == 0){
                    char str[1024] = "Message:";
                    for (int j = tmp_stop;j <= i;j ++){
                        str[8 + j - tmp_stop] = buffer[j];
                    }
                    send(pipe->fd_recv, str,i - tmp_stop + 9, 0);
                }
                else {
                    char str[1024];
                    for (int j = tmp_stop;j <= i;j ++){
                        str[j - tmp_stop] = buffer[j];
                    }
                    send(pipe->fd_recv, str,i - tmp_stop + 1, 0);
                }
                tmp_stop = i + 1;  
                send_flag = 0;        
            }
        }
        if (buffer[len - 1] == '\n') index = 0;
        else {
            for (int i = tmp_stop;i < len;i ++){
                buffer[i - tmp_stop] = buffer[i];
            }
            index = len - tmp_stop;
            
                if (send_flag){
                    send(pipe->fd_recv,buffer,index,0);
                }
                else {
                    char str[1024] = "Message:";
                    for (int j = 0;j < index;j ++){
                        str[8 + j] = buffer[j];
                    }
                    send(pipe->fd_recv, str,index + 8, 0);
                    send_flag = 1;
                }
                index = 0;
            
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    int port = atoi(argv[1]);
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    socklen_t addr_len = sizeof(addr);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind");
        return 1;
    }
    if (listen(fd, 2)) {
        perror("listen");
        return 1;
    }
    int fd1 = accept(fd, NULL, NULL);
    int fd2 = accept(fd, NULL, NULL);
    if (fd1 == -1 || fd2 == -1) {
        perror("accept");
        return 1;
    }
    pthread_t thread1, thread2;
    struct Pipe pipe1;
    struct Pipe pipe2;
    pipe1.fd_send = fd1;
    pipe1.fd_recv = fd2;
    pipe2.fd_send = fd2;
    pipe2.fd_recv = fd1;
    pthread_create(&thread1, NULL, handle_chat, (void *)&pipe1);
    pthread_create(&thread2, NULL, handle_chat, (void *)&pipe2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}
