#include "../kernel/func_test.h"
#include "mem.h"
#include "syscall.h"
#include "start.h"

void kernel_start(void)
{
	// Nettoyage de l'écran
	printf("\f");

	/*************************/
	/* Tests simples horloge */
	/*************************/
	//print_horloge("12:34:56");
	config_horloge();
	init_syscall(49, traitant_IT_49);
	/*************************/
	/* Tests simples process */
	/*************************/

	// Initialisation du processus idle
	processus_t* idle_process = mem_alloc(sizeof(processus_t));
	idle_process->pid = 0;
	idle_process->p_pid = -1;
	idle_process->name[0] = 'i';
	idle_process->name[1] = 'd';
	idle_process->name[2] = 'l';
	idle_process->name[3] = 'e';
	idle_process->name[4] = '\0';
	idle_process->state = ELU;
	idle_process->prio = 0;
	// idle utilise directement la pile noyau, pas besoin d'initialiser regs
	queue_add(idle_process, &queue_process, processus_t, link, prio);

    simple_list_init(&idle_process->children);

	actif = idle_process;
	processus_tab[idle_process->pid] = idle_process;

	/******************/
	/* Tests simples  */
	/******************/
	// start((void*)simple_test, MAX_STACK_SIZE, 128, "test_run", (void*)0);
	/***************************/
	/* Tests complexes process */
	/***************************/
	// start((void*)run_test_proc, MAX_STACK_SIZE, 128, "test_run", (void*)20);

	/****************/
	/* Tests ring 3 */
	/****************/
	int32_t pid = start((void*)user_start, 4096, 128, "user_start", NULL);
	if (pid < 0) {
		printf("[kernel] echec user init\n");
		return;
	}
	// Démarrer le processus par défaut
	idle();

	while(1)
	  hlt();

	return;
}
