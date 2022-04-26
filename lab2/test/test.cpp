#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(){
    int fd = open("test.txt",O_WRONLY | O_CREAT,644);
    write(fd,"hello",5);
    return 0;
}