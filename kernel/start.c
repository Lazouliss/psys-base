#include "../kernel/func_test.h"
#include "mem.h"

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
	// Nettoyage de tout l'écran
	printf("\f");

	/*************************/
	/* Tests simples horloge */
	/*************************/
	//print_horloge("12:34:56");
	config_horloge();
	
	// démasquage des interruptions externes
    //sti(); // Inutile avec l'ajout des processus dynamiques

	/*************************/
	/* Tests simples process */
	/*************************/

	// Initialisation du processus idle
	processus_t* idle_process = mem_alloc(sizeof(processus_t));
	idle_process->pid = 0;
	idle_process->p_pid = -1;
	idle_process->name = "idle";
	idle_process->state = ELU;
	idle_process->prio = 0;
	// idle utilise directement la pile noyau, pas besoin d'initialiser regs
	queue_add(idle_process, &queue_process, processus_t, link, prio);
	
	INIT_LIST_HEAD(&idle_process->children);

	actif = idle_process;
	processus_tab[idle_process->pid] = idle_process;

	start(proc1, MAX_STACK_SIZE, DEFAULT_PRIO, "proc1", NULL);
	start(proc2, MAX_STACK_SIZE, DEFAULT_PRIO, "proc2", NULL);
	start(proc3, MAX_STACK_SIZE, DEFAULT_PRIO, "proc3", NULL);

	// Démarrer le processus par défaut
	idle();
	
	while(1)
	  hlt();

	return;
}
