#include "syscall.h"
#include "stdio.h"
#include "processus.h"

int32_t syscall_PIT(int32_t num, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5)
{
    switch (num) {
        case SYS_GETPID: return getpid();
        default:
            return -1;
    }
    printf("[syscall] num: %d, a1: %d, a2: %d, a3: %d, a4: %d, a5: %d\n", num, a1, a2, a3, a4, a5);
}
