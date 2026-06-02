#include "processus.h"
#include "stdbool.h"
#include "stdio.h"

// initialisation de la table des processus
processus_t processus_table[MAX_PROCESSES];
uint32_t idx_running = 0;
processus_t* actif;

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
