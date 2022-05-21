#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

struct Pipe {
    int fd;
    struct Pipe *next;
};

int thread_index = 0;
struct Pipe *head;

void *handle_chat(void *data) {
    struct Pipe *pipe1 = (struct Pipe *)data;
    char buffer[2000];
    char tmp[1000];
    ssize_t len;
    int index = 0;
    int send_flag = 0;
    int count = 0;
    //int jike = 0;
    //FILE * file = fopen("1_out.txt","w");
    while ((len = recv(pipe1->fd, buffer + index, 1000, 0)) > 0){
// send(pipe1->fd_recv, buffer, len + 8, 0);
        //printf("%d\n",count++);
        //fputs(buffer,file);
        //if (jike) printf("kjsdlk\n");
        int tmp_stop = 0;
        for (int i = 0;i < len;i ++){
            if (buffer[i] == '\n'){
                if (send_flag == 0){
                    char str[1024] = "Message:";
                    for (int j = tmp_stop;j <= i;j ++){
                        str[8 + j - tmp_stop] = buffer[j];
                    }
                    struct Pipe *q = pipe1 -> next;
                    while (q != pipe1){
                        send(q -> fd, str,i - tmp_stop + 9, 0);
                        q = q -> next;
                    }
                }
                else {
                    char str[1024];
                    for (int j = tmp_stop;j <= i;j ++){
                        str[j - tmp_stop] = buffer[j];
                    }
                    struct Pipe *q = pipe1 -> next;
                    while (q != pipe1){
                        send(q -> fd, str,i - tmp_stop + 1, 0);
                        q = q -> next;
                    }
                }
                tmp_stop = i + 1;  
                send_flag = 0;   
                //close(file);     
            }
        }
        if (buffer[len - 1] == '\n') {index = 0;}
        else {
            for (int i = tmp_stop;i < len;i ++){
                buffer[i - tmp_stop] = buffer[i];
            }
            index = len - tmp_stop;
            if (index >= 500){
                if (send_flag){
                    struct Pipe *q = pipe1 -> next;
                    while (q != pipe1){
                        send(q -> fd, buffer,index, 0);
                        q = q -> next;
                    }
                }
                else {
                    char str[1024] = "Message:";
                    for (int j = 0;j < index;j ++){
                        str[8 + j] = buffer[j];
                    }
                    struct Pipe *q = pipe1 -> next;
                    while (q != pipe1){
                        send(q->fd, str,index + 8, 0);
                        q = q -> next;
                    }
                    send_flag = 1;
                }
                index = 0;
            }
        }
    }
    if (head == pipe1){
        head = pipe1 -> next;
    }
    struct Pipe *q = pipe1;
    while (q -> next != pipe1){
        q = q -> next;
    }
    q -> next = pipe1 -> next;
    thread_index --;
    printf ("1");
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

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

    pthread_t thread[32];
    head = malloc(sizeof(struct Pipe));
    int fd_new;
    fd_new = accept(fd,NULL,NULL);
    if (fd_new == -1){
        perror("accept");
        return 1;
    }
    head ->fd = fd_new;
    pthread_mutex_lock(&mutex);
    pthread_create(&thread[thread_index],NULL,handle_chat,(void *)head);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&mutex);

    thread_index ++;
    head -> next = head;

    while (thread_index < 32){
        struct Pipe *pipe = malloc(sizeof(struct Pipe));
        fd_new = accept(fd,NULL,NULL);
        if (fd_new == -1){
            perror("accept");
            return 1;
        }
        pipe -> fd = fd_new; 
        pthread_mutex_lock(&mutex);
        pthread_create(&thread[thread_index],NULL,handle_chat,(void *)pipe);
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mutex);
        pipe -> next = head -> next;
        head -> next = pipe;
        head = pipe;  
        thread_index ++;      
    }
    return 0;
}
