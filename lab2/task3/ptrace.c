#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    pid_t childPid = fork();

    if (childPid < 0) {
        perror("fork failed");
        return 1;
    }
    if (childPid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        raise(SIGSTOP);
        printf("Child message\n");
        getpid();
        return 0;
    }
    int status;
    waitpid(childPid, &status, 0);

    ptrace(PTRACE_SETOPTIONS, childPid, 0, PTRACE_O_TRACESYSGOOD);

    int isSyscallEntry = 1;

    while (1) {
        ptrace(PTRACE_SYSCALL, childPid, 0, 0);
        waitpid(childPid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child exited\n");
            return 0;
        }

        if (WIFSTOPPED(status) && WSTOPSIG(status) == (SIGTRAP | 0x80)) {
            if (!isSyscallEntry) {
                struct user_regs_struct regs;
                ptrace(PTRACE_GETREGS, childPid, 0, &regs);
                printf("Syscall: %llu\n", regs.orig_rax);
                // printf("Syscall: %lld\n", regs.rax);
            }
            isSyscallEntry = !isSyscallEntry;
        }
    }
}
