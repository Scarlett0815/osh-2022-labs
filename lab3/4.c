#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/epoll.h>
int fd_total[32];

int main(int argc, char **argv) {
    char buffer[32][1024];
    int send_flag[32];
    int index1 [32];
    for (int i = 0;i < 32;i ++){
        send_flag[i] = 0;
        index1[i] = 0;
    }
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
    fd_set clients;
    struct epoll_event ev, events[32];
    int epollfd,nfds;
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    int index = 0;
    while(1){
        //printf ("%d**\n",send_flag[1]);
        nfds = epoll_wait(epollfd, events, 32, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == fd) {
                fd_total[index] = accept(fd,NULL,NULL);
                if (fd_total[index] == -1) {
                    perror("accept");
                    return 1;
                }
                fcntl(fd_total[index], F_SETFL, fcntl(fd_total[index], F_GETFL, 0) | O_NONBLOCK);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = fd_total[index];
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_total[index],
                            &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
                FD_SET(fd_total[index], &clients);
                index ++;

            }
        }
        FD_ZERO(&clients);
        FD_SET(fd, &clients);
        for (int k = 0;k < index;k ++){
            FD_SET(fd_total[k], &clients);
        }
        if (select(fd_total[index- 1] + 1, &clients, NULL, NULL, NULL) > 0) { // 找出可以读的套接字
            for (int j = 0;j < index;j ++){
                if (FD_ISSET(fd_total[j], &clients)) {
                    int len = 0;
                    while ((len = recv(fd_total[j], buffer[j] + index1[j], 1000, 0))){
                    
                    int tmp_stop = 0;
                    for (int i = 0;i < len;i ++){
                        if (buffer[j][i] == '\n'){
                            if (send_flag[j] == 0){
                                char str[1024] = "Message:";
                                for (int w = tmp_stop;w <= i;w ++){
                                    str[8 + w - tmp_stop] = buffer[j][w];
                                }
                                for (int k = 0;k < index;k ++){
                                    if (k != j)
                                        send(fd_total[k], str,i - tmp_stop + 9, 0);
                                }
                            }
                            else {
                                char str[1024];
                                for (int w = tmp_stop;w <= i;w ++){
                                    str[w - tmp_stop] = buffer[j][w];
                                }
                                for (int k = 0;k < index;k ++){
                                    if (k != j)
                                        send(fd_total[k], str,i - tmp_stop + 1, 0);
                                }
                            }
                            tmp_stop = i + 1;  
                            send_flag[j] = 0;        
                        }
                    }
                    
                    if (buffer[j][len - 1] == '\n') {index1[j] = 0;break;}
                    else {
                        //printf ("%d\n",send_flag[j]);
                        for (int i = tmp_stop;i < len;i ++){
                            buffer[j][i - tmp_stop] = buffer[j][i];
                        }
                        index1[j] = len - tmp_stop;
                        if (send_flag[j]){
                            for (int k = 0;k < index;k ++){
                                if (k != j)
                                    send(fd_total[k], buffer[j],index1[j], 0);
                            }
                            //printf ("%d\n",send_flag[j]);
                        }
                        else {
                            char str[1024] = "Message:";
                            for (int w = 0;w < index1[j];w ++){
                                str[8 + w] = buffer[j][w];
                            }
                            for (int k = 0;k < index;k ++){
                                if (k != j)
                                    send(fd_total[k], str,index1[j] + 8, 0);
                            }
                            send_flag[j] = 1;
                        }
                        index1[j] = 0;
                    
                    }
                    }
                    if (len <= 0){
                        if (j != index - 1){
                            for (int v = j;v < index -1;v ++){
                                fd_total[v] = fd_total[v + 1];
                            }
                        }
                        index --;
                    }
                }
            }
        } 

    }
    return 0;
}