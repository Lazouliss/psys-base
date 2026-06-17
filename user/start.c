#include "start.h"
#include "stdio.h"
#include "syscall.h"
#include "func_test.h"
#include "shell.h"

void user_start(void)
{
    //cons_write(11, "Je demarre\n");
    start(shell, 4000, 128, "shell", (void*)0);
    //run_test_proc(20);
    //start((void*)run_test_proc, 4096, 128, "test_run", (void*)20);
    //cons_write(10, "Je stoppe\n");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-infinite-loop"
        while(1);
#pragma GCC diagnostic pop
}
