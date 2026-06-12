#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "stdint.h"

enum syscall_num {
    SYS_GETPID = 1
};

// fonction assembleur dans traitant.S
extern void traitant_IT_49(void);
extern void return_to_user(void);

int32_t syscall_PIT(int32_t num, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5);

#endif
