#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

struct Pipe {
    int fd_send;
    int fd_recv[31];
};

int fd_total[32];
struct Pipe pipe2[32];
pthread_t thread[32];
void *handle_chat(void *data) {
    struct Pipe *pipe1 = (struct Pipe *)data;
    char buffer[2000];
    char tmp[1000];
    ssize_t len;
    int index = 0;
    int send_flag = 0;
    int count = 0;
    while ((len = recv(pipe1->fd_send, buffer + index, 1000, 0)) > 0){
// send(pipe1->fd_recv, buffer, len + 8, 0);
        //printf("%d\n",count++);
        int tmp_stop = 0;
        for (int i = 0;i < len;i ++){
            if (buffer[i] == '\n'){
                if (send_flag == 0){
                    char str[1024] = "Message:";
                    for (int j = tmp_stop;j <= i;j ++){
                        str[8 + j - tmp_stop] = buffer[j];
                    }
                    for (int k = 0;k < 31;k ++){
                        send(fd_total[pipe1->fd_recv[k]], str,i - tmp_stop + 9, 0);
                    }
                }
                else {
                    char str[1024];
                    for (int j = tmp_stop;j <= i;j ++){
                        str[j - tmp_stop] = buffer[j];
                    }
                    for (int k = 0;k < 31;k ++){
                        send(fd_total[pipe1->fd_recv[k]], str,i - tmp_stop + 1, 0);
                    }
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
            if (index >= 500){
                if (send_flag){
                    for (int k = 0;k < 31;k ++){
                        send(fd_total[pipe1->fd_recv[k]], buffer,index, 0);
                    }
                }
                else {
                    char str[1024] = "Message:";
                    for (int j = 0;j < index;j ++){
                        str[8 + j] = buffer[j];
                    }
                    for (int k = 0;k < 31;k ++){
                        send(fd_total[pipe1->fd_recv[k]], str,index + 8, 0);
                    }
                    send_flag = 1;
                }
                index = 0;
            }
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
    for (int k = 0;k <= 31;k ++){
        int j = 0;
        int index = 0;
        while (j <= 31){
            if (index == k){
                pipe2[k].fd_recv[j] = index + 1;
                index = index + 2;
            }
            else{
                pipe2[k].fd_recv[j] = index;
                index = index + 1;
            }
            j ++;
        }
    }
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
    int i = 0;
    while (i < 16){
        fd_total[i] = accept(fd,NULL,NULL);
        if (fd_total[i] == -1){
            perror("accept");
            return 1;
        }
        pipe2[i].fd_send = fd_total[i];
        pthread_mutex_lock(&mutex);
        pthread_create(&thread[i],NULL,handle_chat,(void *)&pipe2[i]);
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mutex);
        i ++;
    }
    return 0;
}
