#ifndef __FUNC_TEST_H__
#define __FUNC_TEST_H__

#include "../shared/stdio.h"

#include "debugger.h"
#include "cpu.h"
#include "screen.h"
#include "horloge.h"
#include "processus.h"

int fact(int n);

void idle(void);

int proc1(void*);
int proc2(void*);
int proc3(void*);
int proc4(void*);
int proc5(void*);
int proc6(void*);
int proc7(void*);
int proc8(void*);

int run_test_proc(int max_test);

#endif
