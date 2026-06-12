#include "start.h"
#include "stdio.h"
#include "syscall.h"

void user_start(void)
{
    cons_write(1, "A");
    cons_write(1, "B");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-infinite-loop"
        while(1);
#pragma GCC diagnostic pop
}
