#include "start.h"
#include "stdio.h"
#include "syscall.h"

void user_start(void)
{
    cons_write(7, "coucou ");
    cons_write(3, "gab");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-infinite-loop"
        while(1);
#pragma GCC diagnostic pop
}
