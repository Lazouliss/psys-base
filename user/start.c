#include "start.h"
#include "stdio.h"
#include "syscall.h"
#include "func_test.h"
#include "shell.h"

void user_start(void)
{
    int pid = start(shell, 4000, 128, "shell", 0);
    if (pid > 0) {
        waitpid(pid, 0);
    }
}
