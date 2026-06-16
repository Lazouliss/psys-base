#include "func_test.h"
#include "processus.h"
#include "horloge.h"

int test_run(int n);
int test20();

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

int run_test_proc(int max_test) {
  int first_test = 1;

	for (int i = first_test; i <= max_test; i++) {
		printf("Test %d : ", i);
		test_run(i);
	}

  return 0;
}

int proc1(void*) {
  int count = 0;
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           getpid());
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
           getpid());
    wait_clock(3);
  }
  return 2;
}

int proc3(void*) {
  for (;;) {
    printf("[temps = %lu] processus %s pid = %i\n", current_clock(), mon_nom(),
           getpid());
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
         getpid(), ret);
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
         getpid(), waitpid(proc7_pid, &ret));
  printf("[temps = %lu] processus %s pid = %i, ret = %i\n", current_clock(), mon_nom(),
         getpid(), ret);
  return 0;
}

int proc7(void*) {
  wait_clock(5);

  return 7;
}

void simple_test(void*) {
	int i;
	// call_debugger(); useless with qemu -s -S
	i = 10;
	i = fact(i);

	/************************/
	/* Tests simples printf */
	/************************/
	//efface_ecran();
	// Supprime le contenu de l'écran de base
	printf("\f");
	// Affiche un simple texte qui disparaitra
	printf("Hello, World!\n");
	// Affiche le résultat contenu dans la variable i
	printf("10! = %d\n", i);
	// Test tabulation
	printf("tab\tulation\n");
	// Test defilement()
	printf("ééé\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nderniere lignes");
	// Test place_curseur() au milieu de l'écran
	place_curseur(12, 40);
	// Reviens au début de la ligne, puis écrit 5 caractères, et place le curseur SUR le dernier caractère
	printf("\r klfd\b");
	// Nettoyage de tout l'écran
	printf("\f");

	// démasquage des interruptions externes
    //sti(); // Inutile avec l'ajout des processus dynamiques
}
