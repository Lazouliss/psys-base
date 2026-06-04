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

int proc1(void*) {
  int count = 0;
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(2);
    count++;
    if(count == 3) {
      // Test de la fonction kill sur le processus 3
      printf("Résultat du kill de 3 : %i\n", kill(3));
    }
  }
  return 0;
}

int proc2(void*) {
  for (int i = 0; i < 2; i++) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(3);
  }
  return 0;
}

int proc3(void*) {
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(5);
  }
  return 0;
}

int proc4(void*) {
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(8);
  }
  return 0;
}

int proc5(void*) {
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(13);
  }
  return 0;
}

int proc6(void*) {
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(21);
  }
  return 0;
}

int proc7(void*) {
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(34);
  }
  return 0;
}
