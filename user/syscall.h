#ifndef __USER_SYSCALL_H__
#define __USER_SYSCALL_H__

#include "../shared/syscall_numbers.h"

int chprio(int pid, int newprio);
void cons_write(unsigned long size, const char *str);
int cons_read(unsigned long const size, char str[static size]);
void cons_echo(int on);
__attribute__((noreturn))
void exit(int retval);
int getpid(void);
int getprio(int pid);
int kill(int pid);
int pcount(int fid, int count[static 1]);
int pcreate(int count);
int pdelete(int fid);
int preceive(int fid,int *message);
int preset(int fid);
int psend(int fid, int message);
void clock_settings(unsigned long quartz[static 1], unsigned long ticks[static 1]);
unsigned long current_clock(void);
void wait_clock(unsigned long wakeup);
int start(int (*ptfunc)(void *), unsigned long ssize, int prio, const char *name, void *arg);
int waitpid(int pid, int *retval);
void sys_info(void);
void ps(void);

#endif
