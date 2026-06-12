#ifndef __USER_SYSCALL_H__
#define __USER_SYSCALL_H__

#include "../shared/syscall_numbers.h"

void cons_write(unsigned long size, const char *str);

#endif
