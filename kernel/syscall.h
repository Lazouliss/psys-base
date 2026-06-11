#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "stdint.h"

enum syscall_num {
    SYS_GETPID = 1
};

// fonction assembleur dans syscall.S
extern void _syscall_entry(void);

#endif
