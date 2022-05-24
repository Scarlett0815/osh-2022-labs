#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

struct Str_news{
    char str_small[2000];
    struct Str_news *next;
};

struct Pipe {
    int fd;
    struct Str_news *str_wait;
    struct Pipe *next;
};

int thread_index = 0;
struct Pipe *head;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;


void *handle_send(void *data){
    struct Pipe *pipe1 = (struct Pipe *)data;
    int send_flag = 0;
    int index = 0;
    while (1){
        index = 0;
        if (pipe1 -> str_wait != NULL){
            char buffer[10240];
            struct Str_news *str1 = pipe1 -> str_wait;
            strcpy(buffer,pipe1->str_wait->str_small);   
            int len = strlen(pipe1->str_wait->str_small);
            int tmp_stop = 0;
            for (int i = 0;i < len;i ++){
                if (buffer[i] == '\n'){
                    if (send_flag == 0){
                        char str[10240] = "Message:";
                        for (int j = tmp_stop;j <= i;j ++){
                            str[8 + j - tmp_stop] = buffer[j];
                        } 
                        send(pipe1 -> fd, str,i - tmp_stop + 9, 0);
                    }
                    else {
                        char str[10240];
                        for (int j = tmp_stop;j <= i;j ++){
                            str[j - tmp_stop] = buffer[j];
                        }
                        send(pipe1 -> fd, str,i - tmp_stop + 1, 0);
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
                if (send_flag){
                    send(pipe1 -> fd, buffer,index, 0);
                }
                else {
                    char str[10240] = "Message:";
                    for (int j = 0;j < index;j ++){
                        str[8 + j] = buffer[j];
                    }
                    send(pipe1->fd, str,index + 8, 0);
                    send_flag = 1;
                }
                index = 0;
            }
            pipe1 -> str_wait = str1 -> next;
        }
    }
}

void *handle_chat(void *data) {
    struct Pipe *pipe1 = (struct Pipe *)data;
    char buffer[2000];
    ssize_t len;
    int index = 0;
    int send_flag = 0;
    int count = 0;
    //int jike = 0;
    //FILE * file = fopen("1_out.txt","w");
    while ((len = recv(pipe1->fd, buffer, 1000, 0)) > 0){
        struct Str_news *str = malloc(sizeof(struct Str_news));
        struct Pipe *q = pipe1 -> next;
        strcat(str->str_small,buffer);
        printf ("%s",buffer);
        pthread_mutex_lock(&mutex);
        while (q != pipe1){
            struct Str_news *tail_search = q -> str_wait;
            while (tail_search != NULL && tail_search -> next != NULL){ 
                tail_search = tail_search -> next;
            }
            if (tail_search == NULL){
                q -> str_wait = str;
                str -> next = NULL;
            }
            else{
                tail_search -> next = str;
                str -> next = NULL;
            }
            q = q->next;
        }
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mutex);
        memset(buffer,0,2000);
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
    //printf ("1");
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



    pthread_t thread[32];
    pthread_t thread1[32];
    head = malloc(sizeof(struct Pipe));
    int fd_new;
    fd_new = accept(fd,NULL,NULL);
    if (fd_new == -1){
        perror("accept");
        return 1;
    }
    head ->fd = fd_new;
    
    pthread_create(&thread[thread_index],NULL,handle_chat,(void *)head);
    pthread_create(&thread1[thread_index],NULL,handle_send,(void *)head);
    

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
        pipe->str_wait = NULL;
        pthread_create(&thread[thread_index],NULL,handle_chat,(void *)pipe);
        pthread_create(&thread1[thread_index],NULL,handle_send,(void *)pipe);
        pipe -> next = head -> next;
        head -> next = pipe;
        head = pipe;  
        thread_index ++;      
    }
    return 0;
}
