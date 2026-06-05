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
  return 2;
}

int proc3(void*) {
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           mon_pid());
    wait_clock(5);
  }
  return 0;
}

// proc5 enfant termine avant proc4 parent qui doit le tuer ensuite
int proc4(void*) {
  int proc5_pid = start(proc5, MAX_STACK_SIZE, DEFAULT_PRIO, "proc5", NULL);
  int ret;
  wait_clock(1);
  waitpid(proc5_pid, &ret);
  printf("[temps = %lu] processus %s pid = %i, ret = %i\n", current_clock(), mon_nom(),
         mon_pid(), ret);
  return 0;
}

int proc5(void*) {
  return 2;
}

// proc6 parent qui attend proc7 enfant qui se termine après 5 secondes, et retourne 7 à son parent
int proc6(void*) {
 	int proc7_pid = start(proc7, MAX_STACK_SIZE, DEFAULT_PRIO, "proc7", NULL);
  int ret = 0;
  printf("[temps = %lu] processus %s pid = %i, attend le processus de pid = %d \n", current_clock(), mon_nom(),
         mon_pid(), waitpid(proc7_pid, &ret));
  printf("[temps = %lu] processus %s pid = %i, ret = %i\n", current_clock(), mon_nom(),
         mon_pid(), ret);
  return 0;
}

int proc7(void*) {
  wait_clock(5);

  return 7;
}
