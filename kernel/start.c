#include "debugger.h"
#include "cpu.h"
#include "../shared/stdio.h"
#include "../kernel/screen.h"
#include "../kernel/horloge.h"
#include "../kernel/processus.h"

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
	for (int i = 0; i < 3; i++) {
		printf("[idle] je tente de passer la main a proc1...\n");
		ctx_sw(processus_table[0].registers, processus_table[1].registers);
		printf("[idle] proc1 m'a redonne la main\n");
	}
	printf("[idle] je bloque le systeme\n");
	hlt();
}

void proc1(void) {
	for (;;) {
		printf("[proc1] idle m'a donne la main\n");
		printf("[proc1] je tente de lui la redonner...\n");
		ctx_sw(processus_table[1].registers, processus_table[0].registers);
	}
}

void kernel_start(void)
{
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

	/*************************/
	/* Tests simples horloge */
	/*************************/
	//print_horloge("12:34:56");
	config_horloge();
	
	// démasquage des interruptions externes
    sti();

	/*************************/
	/* Tests simples process */
	/*************************/
	// Initialisation du processus idle
	processus_table[0].pid = 0;
	processus_table[0].name = "idle";
	processus_table[0].state = RUNNING;
	// idle utilise directement la pile noyau, pas besoin d'initialiser regs

	// Initialisation du processus proc1
	processus_table[1].pid = 1;
	processus_table[1].name = "proc1";
	processus_table[1].state = READY;
	// Placer l'adresse de proc1 en sommet de pile et initialiser %esp
	processus_table[1].stack[MAX_STACK_SIZE - 1] = (uint32_t)proc1;
	processus_table[1].registers[1] = (uint32_t)&processus_table[1].stack[MAX_STACK_SIZE - 1];

	// Démarrer le processus par défaut
	idle();
	
	while(1)
	  hlt();

	return;
}
