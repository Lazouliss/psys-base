#include "syscall.h"
#include "stdio.h"
#include "processus.h"
#include "../shared/console.h"
#include "horloge.h"
#include "message.h"

int32_t syscall_PIT(int32_t num, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5)
{
    switch (num) {
        case SYS_CONS_WRITE: cons_write((unsigned long)a1, (const char *)a2); return 0;
        case SYS_EXIT: exit(a1); return 0;
        case SYS_GETPID: return getpid();
        case SYS_GETPRIO: return getprio(a1);
        case SYS_KILL: return kill(a1);
        case SYS_PCOUNT: return pcount(a1, (int *)a2);
        case SYS_PCREATE: return pcreate(a1);
        case SYS_PDELETE: return pdelete(a1);
        case SYS_PRECEIVE: return preceive(a1, (int*)a2);
        case SYS_PRESET: return preset(a1);
        case SYS_PSEND: return psend(a1, a2);
        case SYS_CLOCK_SETTINGS: clock_settings((unsigned long *)a1, (unsigned long *)a2); return 0;
        case SYS_CLOCK: return (int32_t)current_clock();
        case SYS_WAIT_CLOCK: wait_clock((uint32_t)a1); return 0;
        case SYS_START: return start((int (*)(void *))a1, (unsigned long)a2, a3, (const char *)a4, (void *)a5);
        case SYS_WAITPID: return waitpid(a1, (int *)a2);
        default:
            return -1;
    }
}