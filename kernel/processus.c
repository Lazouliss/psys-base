#include "processus.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"
#include "horloge.h"

// initialisation de la table des processus
link queue_process = LIST_HEAD_INIT(queue_process);
link queue_process_sleeping = LIST_HEAD_INIT(queue_process_sleeping);

processus_t* processus_tab[NBPROC];
processus_t* actif;

int32_t last_pid = 0;

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

int kill(int pid) {
    if (pid < 0 || pid >= NBPROC || !processus_tab[pid]) {
        return -1; // PID invalide ou processus déjà terminé
    }
    processus_t* proc = processus_tab[pid];

    mem_free(proc, sizeof(proc)); // Libère la mémoire du processus
    queue_del(proc, link); // Retirer le processus de sa queue actuelle
    processus_tab[pid] = NULL; // Libère la case
    return 0;
}

void exit(int retval) {
    
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-BLA-BLA-BLA"    
        while(true);
#pragma GCC diagnostic pop
    /*
    // recuperer le processus top (courant), le marquer comme zombie, rendre la main, puis le supprimer
    uint32_t pid = actif->pid;
    actif->state = ZOMBIE;
    actif->prio = 0;

    ordonnance();
    printf("Processus %u termine.\n", pid);
    kill(pid);
    */
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
        if ((long unsigned)(-(proc->time_to_wake)) > current_clock()) {
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

const char* mon_nom() {
    return actif->name;
}

int32_t mon_pid() {
    return actif->pid;
}

int getprio(int pid){
    if (pid < 0 || pid >= NBPROC || !processus_tab[pid] || processus_tab[pid]->state == ZOMBIE) {
        return -1;
    }
    return processus_tab[pid]->prio;
}

int chprio(int pid, int newprio) {
    if (newprio < 0 || newprio >= MAX_PRIO || 
        pid < 0 || pid >= NBPROC || !processus_tab[pid] || processus_tab[pid]->state == ZOMBIE) {
        return -1;
    }
    processus_tab[pid]->prio = newprio;
    if (processus_tab[pid]->state == ACTIVABLE) {
        // il doit y être replacé selon sa nouvelle priorité.
        queue_del(processus_tab[pid], link);
        queue_add(processus_tab[pid], &queue_process, processus_t, link, prio);
    }
    return 0;
}

int32_t start(int (*pt_func)(void*), [[maybe_unused]] unsigned long ssize_user, int prio, const char* name, void *arg) {
    if (prio < 0 || prio >= MAX_PRIO) {return -1;}
    // Calcul du prochain PID disponible
    for (size_t i = 1; i < NBPROC; i++)
    {
        // Ignorer le PID 0 réservé pour le processus idle    
        if(processus_tab[(i+last_pid) % NBPROC] == NULL) {
            last_pid = (i+last_pid) % NBPROC;
            break;
        }
    }
    
    processus_t* new_processus = mem_alloc(sizeof(processus_t));
    if (!new_processus) {return -1;}

    uint32_t stack_words = ssize_user / sizeof(uint32_t);
    uint32_t stack_size = stack_words > MAX_STACK_SIZE ? MAX_STACK_SIZE : stack_words;

    new_processus->pid = last_pid;
    new_processus->name = name;
    new_processus->state = ACTIVABLE;
    // Placer l'adresse de code en sommet de pile et initialiser %esp
    new_processus->stack[stack_size - 3] = (uint32_t)pt_func;
    new_processus->stack[stack_size - 2] = (uint32_t)end_processus;
    new_processus->stack[stack_size - 1] = (uint32_t)arg;
    new_processus->registers[1] = (uint32_t)&new_processus->stack[stack_size - 3];
    new_processus->prio = prio;

    queue_add(new_processus, &queue_process, processus_t, link, prio);
    processus_tab[new_processus->pid] = new_processus;

    // Lorsqu'un nouveau processus a une prio > a celle du élu, on les switch
    if(new_processus->prio > actif->prio) {
        ordonnance();
    }

    return new_processus->pid;
}

void wait_clock(uint32_t nbr_secs) {
    actif = queue_top(&queue_process, processus_t, link);
    if (!actif) {
        return; // Pas de processus actif
    }
    actif->state = ENDORMI;
    actif->time_to_wake = -(nbr_secs + current_clock()); // temps de réveil = temps actuel + nombre de secondes à dormir

    ordonnance(); // Appeler l'ordonnanceur pour switcher vers un autre processus pendant que celui-ci est endormi
}
