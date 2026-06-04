#include "func_test.h"
#include "processus.h"
#include "horloge.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}

/**************************/
/* Décla function process */
/**************************/
void idle() {
  for (;;) {
    sti(); // usage de sti et cli avant la phase 5 (possible après)
    hlt(); // autre exception possible: le code de vos tests
    cli(); // JAMAIS de sti ou de cli dans le reste de votre kernel
  }
}

void proc1(void) {
  for (;;) {
    printf("[temps = %u] processus %s pid = %i\n", nbr_secondes(), mon_nom(),
           mon_pid());
    wait_clock(2);
  }
}

void proc2(void) {
  for (;;) {
    printf("[temps = %u] processus %s pid = %i\n", nbr_secondes(), mon_nom(),
           mon_pid());
    wait_clock(3);
  }
}

void proc3(void) {
  for (;;) {
    printf("[temps = %u] processus %s pid = %i\n", nbr_secondes(), mon_nom(),
           mon_pid());
    wait_clock(5);
  }
}

void proc4(void) {
  for (;;) {
    printf("[temps = %u] processus %s pid = %i\n", nbr_secondes(), mon_nom(),
           mon_pid());
    wait_clock(8);
  }
}

void proc5(void) {
  for (;;) {
    printf("[temps = %u] processus %s pid = %i\n", nbr_secondes(), mon_nom(),
           mon_pid());
    wait_clock(13);
  }
}

void proc6(void) {
  for (;;) {
    printf("[temps = %u] processus %s pid = %i\n", nbr_secondes(), mon_nom(),
           mon_pid());
    wait_clock(21);
  }
}

void proc7(void) {
  for (;;) {
    printf("[temps = %u] processus %s pid = %i\n", nbr_secondes(), mon_nom(),
           mon_pid());
    wait_clock(34);
  }
}
