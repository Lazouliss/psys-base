#include "processus.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"
#include "horloge.h"

// initialisation de la table des processus
link queue_process = LIST_HEAD_INIT(queue_process);
link queue_process_sleeping = LIST_HEAD_INIT(queue_process_sleeping);

processus_t* actif;

int32_t last_pid = 1;

// used for debugging purposes
void print_queue() {
    processus_t* proc_iter;
    printf("Queue des processus activables :\n");
    queue_for_each(proc_iter, &queue_process, processus_t, link) {
        printf("PID: %u, Name: %s, State: %u, Prio: %d, Time to wake: %u\n", proc_iter->pid, proc_iter->name, proc_iter->state, proc_iter->prio, proc_iter->time_to_wake);
    }
}
void print_queue_sleeping() {
    processus_t* proc_iter;
    printf("Queue des processus endormis :\n");
    queue_for_each(proc_iter, &queue_process_sleeping, processus_t, link) {
        printf("PID: %u, Name: %s, State: %u, Prio: %d, Time to wake: %d\n", proc_iter->pid, proc_iter->name, proc_iter->state, proc_iter->prio, proc_iter->time_to_wake);
    }
}

void ordonnance(void) {
    if (queue_empty(&queue_process)) {
        return; // Pas de processus activable, on reste sur le processus actuel
    }

    // on réveille tout processus endormi dont le temps de réveil est atteint
    processus_t* proc;
    while (!queue_empty(&queue_process_sleeping)) {
        proc = queue_top(&queue_process_sleeping, processus_t, link);
        assert(proc);
        if (-(proc->time_to_wake) > nbr_secondes()) {
            break;
        }

        proc = queue_out(&queue_process_sleeping, processus_t, link);
        assert(proc);
        proc->state = ACTIVABLE;
        proc->time_to_wake = 0;

        queue_add(proc, &queue_process, processus_t, link, prio);
    }

    // récupère la tête uniquement lorsque que le processus est élu ou s'endort
    processus_t* proc_top = queue_top(&queue_process, processus_t, link);
    assert(proc_top);
    if(proc_top->state == ELU || proc_top->state == ENDORMI) {
        actif = queue_out(&queue_process, processus_t, link);
        if (!actif) {return;} // Pas de processus à switcher
        if (actif->state == ENDORMI) {
            queue_add(actif, &queue_process_sleeping, processus_t, link, time_to_wake);
        } else {
            actif->state = ACTIVABLE;
            queue_add(actif, &queue_process, processus_t, link, prio);
        }
    } else {
        // cas où Idle doit passer à l'état ACTIVABLE et rendre la main
        actif->state = ACTIVABLE;
    }
    
    processus_t* new_actif = queue_top(&queue_process, processus_t, link);
    if (!new_actif) {return;} // Pas de processus à switcher
    new_actif->state = ELU;

    processus_t* old_actif = actif;
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
    new_processus->prio = 128; // priorité par défaut

    queue_add(new_processus, &queue_process, processus_t, link, prio);

    return new_processus->pid;
}

void wait_clock(uint32_t nbr_secs) {
    actif = queue_top(&queue_process, processus_t, link);
    if (!actif) {
        return; // Pas de processus actif
    }
    actif->state = ENDORMI;
    actif->time_to_wake = -(nbr_secs + nbr_secondes()); // temps de réveil = temps actuel + nombre de secondes à dormir

    ordonnance(); // Appeler l'ordonnanceur pour switcher vers un autre processus pendant que celui-ci est endormi
}
