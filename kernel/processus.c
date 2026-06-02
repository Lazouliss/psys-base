#include "processus.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"

// initialisation de la table des processus
processus_t* process_activable_head;
processus_t* process_elu_head;

processus_t* process_activable_tail;
processus_t* process_elu_tail;

uint32_t idx_running = 0;
processus_t* actif;

int32_t last_pid = 1;

void ordonnance(void){
    processus_t* old_actif = actif;
    int32_t next_pid = (old_actif->pid + 1) % MAX_PROCESSES;
    processus_t* new_actif = &processus_table[next_pid];

    old_actif->state = ACTIVABLE;
    new_actif->state = ELU;
    actif = new_actif;

    ctx_sw(old_actif->registers, new_actif->registers);
}

char* mon_nom() {
    return actif->name;
}

int32_t mon_pid() {
    return actif->pid;
}

int32_t cree_processus(void (*code)(void), char *nom) {
    
    processus_t* new_processus = mem_alloc(sizeof(processus_t));

    new_processus->pid = last_pid;
    last_pid++;
    new_processus->name = nom;
    new_processus->state = ACTIVABLE;
    // Placer l'adresse de code en sommet de pile et initialiser %esp
    new_processus->stack[MAX_STACK_SIZE - 1] = (uint32_t)code;
    new_processus->registers[1] = (uint32_t)&new_processus->stack[MAX_STACK_SIZE - 1];

    if (!process_activable_head) {
        process_activable_head->link = LIST_HEAD_INIT(new_processus->name);
    } else {
        process_elu_tail->link.queue_add(new_processus, process_activable_head, processus_t, link, pid);
    }

}
