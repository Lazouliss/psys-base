#include "func_test.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}

/**************************/
/* Décla function process */
/**************************/
void idle(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}

void proc1(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}

void proc2(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}

void proc3(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}

void proc4(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}

void proc5(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}

void proc6(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}

void proc7(void) {
  for (;;) {
    printf("[%s] pid = %i\n", mon_nom(), mon_pid());
    for (int32_t i = 0; i < 100 * 1000 * 1000; i++)
      ;
    sti();
    hlt();
    cli();
  }
}
