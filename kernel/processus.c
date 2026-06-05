#include "processus.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"
#include "horloge.h"

// initialisation de la table des processus
link queue_process = LIST_HEAD_INIT(queue_process);
link queue_process_sleeping = LIST_HEAD_INIT(queue_process_sleeping);
link queue_process_zombie = LIST_HEAD_INIT(queue_process_zombie);
link queue_process_blocked = LIST_HEAD_INIT(queue_process_blocked);

processus_t* processus_tab[NBPROC];
processus_t* actif;

int32_t last_pid = 0;

// used for debugging purposes
void print_queues() {
    printf("=== Etat des queues de processus ===\n");
    print_queue();
    print_queue_sleeping();
    print_queue_zombie();
    print_queue_blocked();
    printf("====================================\n");
}
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
void print_queue_zombie() {
    processus_t* proc_iter;
    printf("Queue des processus zombies :\n");
    queue_for_each(proc_iter, &queue_process_zombie, processus_t, link) {
        printf("PID: %u, Name: %s, State: %u, Prio: %d, Time to wake: %d\n", proc_iter->pid, proc_iter->name, proc_iter->state, proc_iter->prio, proc_iter->time_to_wake);
    }
}
void print_queue_blocked() {
    processus_t* proc_iter;
    printf("Queue des processus bloqués :\n");
    queue_for_each(proc_iter, &queue_process_blocked, processus_t, link) {
        printf("PID: %u, Name: %s, State: %u, Prio: %d, Time to wake: %d\n", proc_iter->pid, proc_iter->name, proc_iter->state, proc_iter->prio, proc_iter->time_to_wake);
    }
}

/*
* Supprime le processus identifié par pid de la table des processus et de toutes les queues, et libère sa mémoire.
* 
* int pid : PID du processus à supprimer
* return : 0 si succès, -1 si échec (PID invalide ou processus déjà terminé)
*/
int kill(int pid) {
    if (pid < 0 || pid >= NBPROC || !processus_tab[pid]) {
        return -1; // PID invalide ou processus déjà terminé
    }
    //printf("Tentative de suppression du processus %s.\n", processus_tab[pid]->name);
    processus_t* proc = processus_tab[pid];

    queue_del(proc, link); // Retirer le processus de sa queue actuelle
    // TODO: free children / siblings
    mem_free(proc, sizeof(processus_t)); // Libère la mémoire du processus
    processus_tab[pid] = NULL; // Libère la case
    return 0;
}

/*
* Wrapper d'exécution d'un processus. Lance une fonction puis la termine avec un appelle à exit()
*
* int (*pt_func)(void*) : pointeur vers la fonction à exécuter
* int* arg : argument à passer dans la fonction pt_func
* return : rien, termine par une exécution de la fonction exit()
*/
void run_process_exec(int (*pt_func)(void*), int* arg) {
    assert(pt_func != NULL);
    exit(pt_func(arg));
}

/*
* Termine le processus courant en lui assignant un code de retour, et effectue les opérations nécessaires 
* pour le marquer comme terminé et permettre à son père de récupérer sa valeur de retour.
* 
* int retval : code de retour du processus, qui sera récupéré par le père lors d'un appel à waitpid
* return : ne retourne jamais, le processus se termine et est nettoyé par son père ou par l'ordonnanceur
*/
__attribute__((noreturn))
void exit(int retval) {
    //printf("Processus %u termine avec le code de retour %d.\n", actif->pid, retval);
    // set to ZOMBIE only if not BLOCK_CHILD
    if (actif->state != BLOCK_CHILD) {
        actif->state = ZOMBIE;
    }
    actif->retval = retval;

    // débloque son pere et lui rend la main, s'il est en BLOCK_CHILD
    // Sinon le fils est en ZOMBIE et regarde que son pere l'attend (parent->blocking_cid) pour lui mettre sa retval dans parent->retval, et finir par se suicider (mettre son parent ELU et se plonger dans la poubelle). (exit)
    processus_t* parent = processus_tab[actif->p_pid];
    if (parent && parent->state == BLOCK_CHILD 
        && ((uint32_t)parent->blocking_cid == actif->pid || parent->blocking_cid == -1)) {
        parent->state = ACTIVABLE;
        parent->blocking_cid = 0;
        parent->retval = retval;
        actif->state = DYING;
    }
    ordonnance();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-infinite-loop"    
        while(true);
#pragma GCC diagnostic pop
}

/*
* Permet à un processus père d'attendre la fin d'un de ses processus fils, de récupérer sa valeur de retour, et de libérer les ressources associées à ce processus fils.
* 
* int pid : PID du processus fils à attendre, ou -1 pour attendre n'importe quel processus fils
* int *retvalp : pointeur vers une variable où sera stockée la valeur de retour du processus fils terminé
* return : PID du processus fils terminé, ou -1 en cas d'erreur (PID invalide, processus fils non enfant, ou aucun processus fils n'est encore terminé)
*/
int waitpid(int pid, int *retvalp) {
    if (simple_list_empty(actif->children) || pid > NBPROC) { return -1;}

    // regarde ses enfants, att qu'ils deviennent zombie, recupere leur valeur retval, les tue et retourne leur pid
    if (pid < 0) {
        processus_t* proc_iter;
        simple_list_for_each(proc_iter, actif->children, processus_t, siblings) {
            if(proc_iter->state == ZOMBIE) {
                if(retvalp) {
                    *retvalp = proc_iter->retval;
                }
                int ret_kill = kill(proc_iter->pid);
                assert(ret_kill == 0);
                return proc_iter->pid;
            }
        }
        //Si le fils n'est pas pret, le parent est blocké par le pid de son fils. 
        actif->state = BLOCK_CHILD;
        actif->blocking_cid = -1;       // on attend n'importe quel fils
        ordonnance(); // Attendre que l'un des processus enfants devienne zombie
        return -1;
    } 

    processus_t* child = processus_tab[pid];
    if (!child || child->p_pid != actif->pid) {
        return -1; // PID invalide ou processus non enfant
    }
    //Si le fils n'est pas pret, le parent est blocké par le pid de son fils. 
    if (child->state != ZOMBIE) {
        actif->state = BLOCK_CHILD;
        actif->blocking_cid = pid;
        ordonnance(); // Attendre que le processus enfant devienne zombie
        return -1;
    }

    //Dans le cas ou l'enfant est fini (ZOMBIE), mais que son parent n'est pas bloqué, alors il reste dans la queue_process_zombie, et sera nettoyé par son père lorsqu'il récupérera sa valeur ret.
    if (!child->retval) { return -1; }
    *retvalp = child->retval;

    int ret_kill = kill(child->pid);
    if (ret_kill != 0) { return -1; }

    return pid;
}

/* 
* L'ordonnanceur est responsable de la gestion des processus, en décidant quel processus doit être exécuté à un moment donné.
* Il gère les transitions d'état des processus (activable, élu, endormi, bloqué, zombie, etc.) et effectue les changements de contexte nécessaires pour passer d'un processus à un autre.
* Termine par un changement de contexte (ctx_sw)
*/
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

    // Clean dying list, les processus ont fini leur boulot    
    processus_t* proc_iter_zombie;
    queue_for_each(proc_iter_zombie, &queue_process_zombie, processus_t, link) {
        //printf("ZOMBIE : proc %s avec un state : %i\n", proc_iter_zombie->name, proc_iter_zombie->state);
        if(proc_iter_zombie->state == DYING) {
            int ret_kill = kill(proc_iter_zombie->pid);
            assert(ret_kill == 0);
            break;
        }
    }

    // Liberer les processus bloqués
    processus_t* proc_iter_blocked;
    queue_for_each(proc_iter_blocked, &queue_process_blocked, processus_t, link) {
        //printf("BLOCKED : proc %s avec un state : %i\n", proc_iter_blocked->name, proc_iter_blocked->state);
        if(proc_iter_blocked->state == ACTIVABLE) {
            queue_del(proc_iter_blocked, link);
            queue_add(proc_iter_blocked, &queue_process, processus_t, link, prio);
            break;
        }
    }

    // récupère la tête uniquement lorsque que le processus est élu ou s'endort
    processus_t* proc_top = queue_top(&queue_process, processus_t, link);
    assert(proc_top);

    if (proc_top->state == ELU || proc_top->state == ENDORMI || proc_top->state == ZOMBIE || proc_top->state == DYING || proc_top->state == BLOCK_CHILD) {
        actif = queue_out(&queue_process, processus_t, link);
        if (!actif) {return;} // Pas de processus à switcher
        switch(actif->state) {
            case ENDORMI:
                queue_add(actif, &queue_process_sleeping, processus_t, link, time_to_wake);
                break; // On garde le processus endormi dans la queue des processus endormis
            case ZOMBIE:
            case DYING:
                // Si le proc est ZOMBIE on doit le deplacer dans la liste des zombies
                queue_add(actif, &queue_process_zombie, processus_t, link, prio);
                break;
            case BLOCK_CHILD:
                // si actif est BLOCK_CHILD on le place temporairement dans la queue des processus non executables
                queue_add(actif, &queue_process_blocked, processus_t, link, prio);
                break;
            default:
                // Cas ELU ou autre, on le remet dans la queue des processus en ACTIVABLE
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

/*
* Crée un nouveau processus et le démarre si sa priorité est supérieure à celle du processus actif.
* 
* int (*pt_func)(void*) : pointeur vers la fonction à exécuter dans le processus, qui doit prendre un argument de type void* et retourner un int
* unsigned long ssize_user : taille de la pile utilisateur à allouer pour le processus, en octets
* int prio : priorité du processus, un entier entre 0 (priorité la plus basse) et MAX_PRIO (priorité la plus haute)
* const char* name : nom du processus, une chaîne de caractères pour l'identification et le débogage
* void *arg : argument à passer à la fonction pt_func lors de son exécution
* return : PID du processus créé, ou -1 en cas d'erreur (par exemple, si la table des processus est pleine ou si les paramètres sont invalides)
*/
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

    //uint32_t stack_words = ssize_user / sizeof(uint32_t);
    //uint32_t stack_size = stack_words > MAX_STACK_SIZE ? MAX_STACK_SIZE : stack_words;

    new_processus->pid = last_pid;
    new_processus->name = name;
    new_processus->state = ACTIVABLE;
    // Placer l'adresse de code en sommet de pile et initialiser %esp
    new_processus->stack[MAX_STACK_SIZE - 4] = (uint32_t)run_process_exec;
    new_processus->stack[MAX_STACK_SIZE - 2] = (uint32_t)pt_func;
    new_processus->stack[MAX_STACK_SIZE - 1] = (uint32_t)arg;
    new_processus->registers[0] = 0;
    new_processus->registers[1] = (uint32_t)&new_processus->stack[MAX_STACK_SIZE - 4];  // %esp -> wrapper d'exécution
    new_processus->registers[2] = (uint32_t)&new_processus->stack[MAX_STACK_SIZE - 3];  // %ebp -> valeur de retour du wrapper
    new_processus->registers[3] = 0;
    new_processus->registers[4] = 0;
    new_processus->prio = prio;
    // Filiation
    new_processus->p_pid = actif->pid;
    simple_list_init(&new_processus->children);
    simple_list_add(new_processus, &actif->children, processus_t, siblings); // Ajouter le processus à la liste des enfants de son père

    // Ajouter le processus à la table + queue
    queue_add(new_processus, &queue_process, processus_t, link, prio);
    processus_tab[new_processus->pid] = new_processus;

    // Lorsqu'un nouveau processus a une prio > a celle du élu, on les switch
    if(new_processus->prio > actif->prio) {
        ordonnance();
    }

    return new_processus->pid;
}

/*
* TODO: mettre à jour pour suivre la SPEC -> s'arrêter au bout de nbr_secs secondes depuis le lancement du programme, et pas depuis l'appel à wait_clock
* Permet au processus actif de se mettre en sommeil pendant un certain nombre de secondes.
* 
* uint32_t nbr_secs : nombre de secondes pendant lesquelles le processus doit être endormi
* return : ne retourne rien, le processus se met en sommeil et rend la main à l'ordonnanceur
*/
void wait_clock(uint32_t nbr_secs) {
    actif = queue_top(&queue_process, processus_t, link);
    if (!actif) {
        return; // Pas de processus actif
    }
    actif->state = ENDORMI;
    actif->time_to_wake = -(nbr_secs + current_clock()); // temps de réveil = temps actuel + nombre de secondes à dormir

    ordonnance(); // Appeler l'ordonnanceur pour switcher vers un autre processus pendant que celui-ci est endormi
}
