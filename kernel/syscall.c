#include "syscall.h"
#include "stdio.h"
#include "processus.h"
#include "../shared/console.h"
#include "horloge.h"
#include "message.h"
#include "screen.h"
#include "keyboard.h"

 // Check when args are pointer, if they are in a valid user space address
bool is_valid_user_space(void* ptr, unsigned long size) {
    if (!ptr) return false;

    uint32_t addr = (uint32_t)ptr;
    return (addr >= 0x01000000 && addr + size < 0x03000000);
}

static void sys_info(void)
{
    printf("=== system info ===\n");
    printf("pid courant: %d\n", getpid());
    printf("temps ticks: %lu\n", current_clock());
    ps();
    list_messages();
}

int32_t syscall_PIT(int32_t num, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5)
{
    switch (num) {
        case SYS_CONS_WRITE:
            if (!is_valid_user_space((void *)a2, (unsigned long)a1)) { return -1; }
            cons_write((unsigned long)a1, (const char *)a2);
            return 0;
        case SYS_EXIT: exit(a1); return 0;
        case SYS_GETPID: return getpid();
        case SYS_GETPRIO: return getprio(a1);
        case SYS_KILL: return kill(a1);
        case SYS_PCOUNT:
            if (!is_valid_user_space((void *)a2, sizeof(int))) { return -1; }
            return pcount(a1, (int *)a2);
        case SYS_PCREATE: return pcreate(a1);
        case SYS_PDELETE: return pdelete(a1);
        case SYS_PRECEIVE:
            if (a2 != 0 && !is_valid_user_space((void *)a2, sizeof(int))) { return -1; }
            return preceive(a1, (int*)a2);
        case SYS_PRESET: return preset(a1);
        case SYS_PSEND: return psend(a1, a2);
        case SYS_CLOCK_SETTINGS:
            if (!is_valid_user_space((void *)a1, sizeof(unsigned long)) || !is_valid_user_space((void *)a2, sizeof(unsigned long))) { return -1; }
            clock_settings((unsigned long *)a1, (unsigned long *)a2);
            return 0;
        case SYS_CLOCK: return (int32_t)current_clock();
        case SYS_WAIT_CLOCK:
            wait_clock((uint32_t)a1);
            return 0;
        case SYS_START:
            if (!is_valid_user_space((void *)a1, 1) || !is_valid_user_space((void *)a4, NAME_MAX_LEN)) { return -1; }
            return start((int (*)(void *))a1, (unsigned long)a2, a3, (const char *)a4, (void *)a5);
        case SYS_WAITPID:
            if (a2 != 0 && !is_valid_user_space((void *)a2, sizeof(int))) { return -1; }
            return waitpid(a1, (int *)a2);
        case SYS_PLC_CURS: place_curseur((uint32_t)a1, (uint32_t)a2); return 0;
        case SYS_DEFILEMENT: defilement(); return 0;
        case SYS_CHPRIO: return chprio(a1, a2);
        case SYS_CONS_READ:
            if (!is_valid_user_space((void *)a2, (unsigned long)a1)) { return -1; }
            return cons_read((unsigned long)a1, (char *)a2);
        case SYS_CONS_ECHO: cons_echo(a1); return 0;
        case SYS_INFO: sys_info(); return 0;
        case SYS_PS: ps(); return 0;
        case SYS_CHANGE_COLORS:
            if (a1 < 0 || a1 > 7 || a2 < 0 || a2 > 15) { return -1; }
            change_colors((uint8_t)a1, (uint8_t)a2);
            return 0;
        default:
            return -1;
    }
}
