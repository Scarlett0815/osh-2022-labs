#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <linux/kernel.h>
#include <errno.h>
int main()
{
	char buffer[200];
	long signal = syscall(548,buffer,sizeof(buffer));
	if (!signal){
		printf("return %ld,buffer = %s",signal,buffer);
	}
	else{
		printf("return %ld\n",signal);
	}
	return 0;
}
