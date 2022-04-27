#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h> /* for ptrace() */
#include <sys/user.h> 
#include <sys/wait.h>


int main ( int argc, char * argv[] )
{
 int status; 
 pid_t pid;
 struct user_regs_struct regs;
 int counter = 0;
 int in_call =0;
 
 switch(pid = fork()){
   case -1: 
        perror("fork");
        exit(1);
   case 0: /* in the child process */
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1],argv+1);
   default: /* in the parent process */
        waitpid(pid,0,0);
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL); 
        waitpid(pid,0,0);
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);
        long syscall = regs.orig_rax;
        fprintf(stderr, "%ld(%ld, %ld, %ld, %ld, %ld, %ld)\n", syscall,
        (long)regs.rdi, (long)regs.rsi, (long)regs.rdx,
        (long)regs.r10, (long)regs.r8,  (long)regs.r9
        ); 
   }
   //fprintf(stderr, " = %ld\n", (long)regs.rax);
   return 0; 
}