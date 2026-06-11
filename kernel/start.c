#include "../kernel/func_test.h"
#include "mem.h"
#include "syscall.h"
#include "start.h"

void kernel_start(void)
{
	/*************************/
	/* Tests simples horloge */
	/*************************/
	//print_horloge("12:34:56");
	config_horloge();
	init_traitant_IT(0x49, _syscall_entry);
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
	int32_t pid = start((void*)user_start, 0, 128, "user_start", NULL);
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
