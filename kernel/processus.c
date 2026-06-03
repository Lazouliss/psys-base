#include "processus.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"

// initialisation de la table des processus
link queue_process = LIST_HEAD_INIT(queue_process);

uint32_t idx_running = 0;

int32_t last_pid = 1;

void ordonnance(void) {
    if (queue_empty(&queue_process)) {
        return; // Pas de processus activable, on reste sur le processus actuel
    }

    processus_t* old_actif = queue_out(&queue_process, processus_t, link);
    if (!old_actif) {return;} // Pas de processus à switcher
    old_actif->state = ACTIVABLE;
    queue_add(old_actif, &queue_process, processus_t, link, prio);
    
    processus_t* new_actif = queue_top(&queue_process, processus_t, link);
    if (!new_actif) {return;} // Pas de processus à switcher
    new_actif->state = ELU;

    ctx_sw(old_actif->registers, new_actif->registers);
}

char* mon_nom() {
    processus_t* actif = queue_top(&queue_process, processus_t, link);
    if (!actif) {
        return ""; // Pas de processus actif
    }
    return actif->name;
}

int32_t mon_pid() {
    processus_t* actif = queue_top(&queue_process, processus_t, link);
    if (!actif) {
        return -1; // Pas de processus actif
    }
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

    queue_add(new_processus, &queue_process, processus_t, link, prio);

    return new_processus->pid;
}
/*
void dors(uint32_t nbr_secs) {

}
*/