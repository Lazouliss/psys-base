#include "processus.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"

// initialisation de la table des processus
link queue_process_activable = LIST_HEAD_INIT(queue_process_activable);
link queue_process_elu = LIST_HEAD_INIT(queue_process_elu);

uint32_t idx_running = 0;
processus_t* actif;

int32_t last_pid = 1;

void ordonnance(void) {
    if (queue_empty(&queue_process_activable)) {
        return; // Pas de processus activable, on reste sur le processus actuel
    }

    processus_t* old_actif = queue_out(&queue_process_elu, processus_t, link);
    processus_t* new_actif = queue_out(&queue_process_activable, processus_t, link);

    if (!old_actif || !new_actif) {
        return; // Pas de processus à switcher
    }

    old_actif->state = ACTIVABLE;
    new_actif->state = ELU;
    
    actif = new_actif;

    queue_add(old_actif, &queue_process_activable, processus_t, link, prio);
    queue_add(new_actif, &queue_process_elu, processus_t, link, prio);

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
    if (!new_processus || last_pid >= MAX_PROCESSES) {
        return -1;
    }

    new_processus->pid = last_pid;
    last_pid++;
    new_processus->name = nom;
    new_processus->state = ACTIVABLE;
    // Placer l'adresse de code en sommet de pile et initialiser %esp
    new_processus->stack[MAX_STACK_SIZE - 1] = (uint32_t)code;
    new_processus->registers[1] = (uint32_t)&new_processus->stack[MAX_STACK_SIZE - 1];
    new_processus->prio = 0; // priorité par défaut

    queue_add(new_processus, &queue_process_activable, processus_t, link, prio);

    return new_processus->pid;
}
