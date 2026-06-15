#include "processus.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"
#include "horloge.h"
#include "message.h"
#include "user_stack_mem.h"
#include "processor_structs.h"
#include "syscall.h"

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
* Fonction interne pour vraiment libérer la mémoire d'un processus.
*
* processus_t* proc : pointeur vers le processus à libérer
* return : rien, libère la mémoire du processus et le vide de la liste des enfants de son père
*/
static void free_process(processus_t* proc) {
    int pid = proc->pid;
    // Retirer de la liste des enfants de son père
    if (pid != 0 && processus_tab[proc->p_pid]) {
        simple_list_remove(proc, &processus_tab[proc->p_pid]->children, processus_t, siblings);
    }
    // Lorsqu'un parent meurt, il laisse ses enfants activables orphelins, et supprime les zombies/dying qui lui restent
    processus_t* child;
    processus_t* next_child;
    simple_list_for_each(child, proc->children, processus_t, siblings) {
        if (child->state == ZOMBIE || child->state == DYING) {
            // On calcul le next seulement s'il existe
            if (child->link.next != NULL) {
                next_child = list_entry(child->link.next, processus_t, siblings);
            }
            queue_del(child, link);
            free_process(child);
            if (child->link.next != NULL) {
                child = next_child;
            }
        } else {
            child->p_pid = 0; // orphelin
        }
    }

    processus_tab[pid] = NULL;
    mem_free(proc, sizeof(processus_t));
}

/*
* Marque le processus identifié par pid comme ZOMBIE (retval=0).
* Ne libère pas la mémoire : le père doit appeler waitpid pour cela.
*
* int pid : PID du processus à tuer
* return : 0 si succès, -1 si échec (PID invalide, pid==0, ou processus déjà zombie)
*/
int kill(int pid) {
    if (pid <= 0 || pid >= NBPROC || !processus_tab[pid]) {
        return -1; // PID invalide
    }
    processus_t* proc = processus_tab[pid];
    // Interdire de tuer un processus déjà zombie/dying
    if (proc->state == ZOMBIE || proc->state == DYING) {
        return -1;
    }
    proc->retval = 0; // retval d'un processus tué est 0
    proc->state = ZOMBIE;

    if(actif->pid == (uint32_t)pid) {
        // Gestion du suicide
        ordonnance(); // Si on tue le processus actif, on doit rendre la main
    } else {
        // Retirer de sa queue courante s'il n'est pas élu, sinon c'est ordonnance qui traite le cas
        queue_del(proc, link);
        // Placer dans la queue des zombies
        queue_add(proc, &queue_process_zombie, processus_t, link, prio);
    }

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
        parent->blocking_cid = actif->pid;  // stocker le pid du fils qui termine
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
    // waitpid sur soi-même est invalide
    if (pid == (int)actif->pid) { return -1; }
    // pid > NBPROC invalide
    if (pid >= NBPROC) { return -1; }

    if (pid < 0) {
        // Attendre n'importe quel fils
        if (simple_list_empty(actif->children)) { return -1; }

        processus_t* proc_iter;
        simple_list_for_each(proc_iter, actif->children, processus_t, siblings) {
            if(proc_iter->state == ZOMBIE) {
                int cpid = proc_iter->pid;
                if(retvalp) {
                    *retvalp = proc_iter->retval;
                }
                queue_del(proc_iter, link);
                free_process(proc_iter);
                return cpid;
            }
        }
        // Aucun fils zombie, on se bloque
        actif->state = BLOCK_CHILD;
        actif->blocking_cid = -1;
        ordonnance(); // Attendre qu'un fils termine

        // Après réveil : le parent récupère les infos de son fils qui termine
        int cpid = actif->blocking_cid;
        actif->blocking_cid = 0;
        if (retvalp) {
            *retvalp = actif->retval;
        }
        // Le fils est en DYING dans queue_zombie, le nettoyer
        processus_t* child = processus_tab[cpid];
        if (child) {
            queue_del(child, link);
            free_process(child);
        }
        return cpid;
    }

    // pid spécifique
    processus_t* child = processus_tab[pid];
    if (!child || child->p_pid != actif->pid) {
        return -1; // PID invalide ou processus non enfant
    }
    //Si le fils n'est pas pret, le parent est blocké par le pid de son fils.
    if (child->state != ZOMBIE) {
        actif->state = BLOCK_CHILD;
        actif->blocking_cid = pid;
        ordonnance(); // Attendre que le processus enfant devienne zombie/dying

        // Après réveil
        actif->blocking_cid = 0;
        if (retvalp) {
            *retvalp = actif->retval;
        }
        // Nettoyer le fils (qui est DYING dans queue_zombie)
        processus_t* c = processus_tab[pid];
        if (c) {
            queue_del(c, link);
            free_process(c);
        }
        return pid;
    }

    // Le fils est déjà ZOMBIE, récupérer retval et le libérer
    if (retvalp) {
        *retvalp = child->retval;
    }
    queue_del(child, link);
    free_process(child);
    return pid;
}

/**
 * Réveille les processus dont le time_to_wake a été dépassé.
 */
void wake_up_processes() {
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
}

/**
 * On libère les processus qui ont fini leur boulot
 * On libère aussi les orphelins DYING dont le père est mort
 */
void clean_zombie_processes() {
    processus_t* proc_iter_zombie;
    processus_t* proc_next_zombie;
    queue_for_each(proc_iter_zombie, &queue_process_zombie, processus_t, link) {
        //printf("ZOMBIE : proc %s avec un state : %i\n", proc_iter_zombie->name, proc_iter_zombie->state);

        // TODO: si son père process_tab[ppid] est mort, on le libère directement, sinon on attend que son père fasse waitpid pour le libérer
        if(processus_tab[proc_iter_zombie->p_pid] == NULL) {
            // Manuellement passer au next link du proc_iter_zombie avant de le supprimer, sinon on perd la référence à la queue et on ne peut plus itérer
            proc_next_zombie = queue_entry(proc_iter_zombie->link.next, processus_t, link);

            queue_del(proc_iter_zombie, link);
            free_process(proc_iter_zombie);

            proc_iter_zombie = proc_next_zombie;
        }

        if(proc_iter_zombie->state == DYING) {
            int ppid = proc_iter_zombie->p_pid;
            if (!processus_tab[ppid] || ppid == 0) {
                // Manuellement passer au next link du proc_iter_zombie avant de le supprimer, sinon on perd la référence à la queue et on ne peut plus itérer
                proc_next_zombie = queue_entry(proc_iter_zombie->link.next, processus_t, link);

                queue_del(proc_iter_zombie, link);
                free_process(proc_iter_zombie);

                proc_iter_zombie = proc_next_zombie;
            }
        }
    }
}

/**
 * Libère les processus bloqués
 */
void unblock_processes() {
    processus_t* proc_iter_blocked;
    processus_t* proc_next_blocked;
    queue_for_each(proc_iter_blocked, &queue_process_blocked, processus_t, link) {
        //printf("BLOCKED : proc %s avec un state : %i\n", proc_iter_blocked->name, proc_iter_blocked->state);
        if(proc_iter_blocked->state == ACTIVABLE) {
            // Manuellement passer au next link du proc_iter_blocked avant de le supprimer, sinon on perd la référence à la queue et on ne peut plus itérer
            proc_next_blocked = queue_entry(proc_iter_blocked->link.next, processus_t, link);

            queue_del(proc_iter_blocked, link);
            queue_add(proc_iter_blocked, &queue_process, processus_t, link, prio);

            proc_iter_blocked = proc_next_blocked;
        }
    }
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
    wake_up_processes();

    // Clean dying list
    clean_zombie_processes();

    // Libere les processus bloqués
    unblock_processes();

    // récupère la tête uniquement lorsque que le processus est élu ou s'endort
    processus_t* proc_top = queue_top(&queue_process, processus_t, link);
    assert(proc_top);

    if (proc_top->state != ACTIVABLE) {
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
            case BLOCK_MSG_RCV:
                // si actif est BLOCK_MSG_RCV on le place dans la queue des receiver
                queue_add(actif, &message_tab[actif->blocking_fid]->receiver_queue, processus_t, link, prio);
                break;
            case BLOCK_MSG_SND:
                // si actif est BLOCK_MSG_SND on le place dans la queue des sender
                queue_add(actif, &message_tab[actif->blocking_fid]->sender_queue, processus_t, link, prio);
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

    // Lorsqu'un processus est de type user, on doit actualiser le pointeur vers la tête de sa stack kernel dans le TSS pour que les interruptions puissent fonctionner correctement
    if (new_actif->is_user) {
        tss.esp0 = (uint32_t)&actif->kernel_stack[MAX_STACK_SIZE];
        tss.ss0 = KERNEL_DS;
    }

    ctx_sw(old_actif->registers, new_actif->registers);
}

const char* mon_nom() {
    return actif->name;
}

int32_t getpid() {
    return actif->pid;
}

int getprio(int pid){
    if (pid < 0 || pid >= NBPROC || !processus_tab[pid] ||
        processus_tab[pid]->state == ZOMBIE || processus_tab[pid]->state == DYING) {
        return -1;
    }
    return processus_tab[pid]->prio;
}

/*
* Change la priorité d'un processus identifié par pid, et réordonne les processus si nécessaire.
*
* int pid : PID du processus dont on veut changer la priorité
* int newprio : nouvelle priorité à assigner au processus, un entier entre 0 et MAX_PRIO
* return : ancienne priorité du processus, ou -1 en cas d'erreur (PID invalide, pid==0, processus zombie/dying, ou nouvelle priorité invalide)
*/
int chprio(int pid, int newprio) {
    if (newprio <= 0 || newprio >= MAX_PRIO ||
        pid < 0 || pid >= NBPROC || !processus_tab[pid] ||
        processus_tab[pid]->state == ZOMBIE || processus_tab[pid]->state == DYING) {
        return -1;
    }
    int old_prio = processus_tab[pid]->prio;
    processus_tab[pid]->prio = newprio;
    if (processus_tab[pid]->state == ACTIVABLE) {
        // il doit y être replacé selon sa nouvelle priorité.
        queue_del(processus_tab[pid], link);
        queue_add(processus_tab[pid], &queue_process, processus_t, link, prio);
    } 
    // Un processus bloqué sur file vide et dont la priorité est changée par chprio, est considéré comme le dernier processus (le plus jeune) de sa nouvelle priorité.
    else if (processus_tab[pid]->state == BLOCK_MSG_RCV || processus_tab[pid]->state == BLOCK_MSG_SND) {
        // Reordonner dans la file de messages selon la nouvelle priorité
        int fid = processus_tab[pid]->blocking_fid;
        if (fid >= 0 && fid < NBQUEUE && message_tab[fid]) {
            link *q = (processus_tab[pid]->state == BLOCK_MSG_SND)
                      ? &message_tab[fid]->sender_queue
                      : &message_tab[fid]->receiver_queue;
            queue_del(processus_tab[pid], link);
            queue_add(processus_tab[pid], q, processus_t, link, prio);
        }
    }
    // Si le processus élu perd de la priorité, on ordonnance
    if (processus_tab[pid]->state == ELU) {
        ordonnance();
    }
    return old_prio;
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
    // Taille de pile maximale en octets
    const unsigned long max_ssize = (unsigned long)MAX_STACK_SIZE * sizeof(uint32_t);
    if (prio <= 0 || prio >= MAX_PRIO) {return -1;}
    if (ssize_user > max_ssize) {return -1;}

    bool space_found = false;
    // Calcul du prochain PID disponible
    for (size_t i = 1; i < NBPROC; i++)
    {
        // Ignorer le PID 0 réservé pour le processus idle
        if(processus_tab[(i+last_pid) % NBPROC] == NULL) {
            last_pid = (i+last_pid) % NBPROC;
            space_found = true;
            break;
        }
    }

    // plus de processus dispo
    if (!space_found) { return -1; }

    processus_t* new_processus = mem_alloc(sizeof(processus_t));
    if (!new_processus) {return -1;}

    // Allocation de la pile kernel / utilisateur
    new_processus->kernel_stack = mem_alloc(sizeof(uint32_t) * MAX_STACK_SIZE);
    if (!new_processus->kernel_stack) {
        mem_free(new_processus, sizeof(processus_t));
        return -1;
    }

    // Initialisation des paramètres commun du processus
    new_processus->pid = last_pid;
    new_processus->name = name;
    new_processus->state = ACTIVABLE;
    new_processus->registers[0] = 0;
    new_processus->registers[1] = 0;
    new_processus->registers[2] = 0;
    new_processus->registers[3] = 0;
    new_processus->registers[4] = 0;
    new_processus->prio = prio;

    if (ssize_user > 0) {
        // Initialisation du processus utilisateur
        new_processus->is_user = true;
        new_processus->user_stack_size = ssize_user;
        new_processus->user_stack = user_stack_alloc(ssize_user);
        if (!new_processus->user_stack) {
            mem_free(new_processus->kernel_stack, sizeof(uint32_t) * MAX_STACK_SIZE);
            mem_free(new_processus, sizeof(processus_t));
            return -1;
        }
        uint32_t user_stack_top = (uint32_t)new_processus->user_stack + ssize_user;

        uint32_t *user_esp = (uint32_t *)user_stack_top;

        // new_processus->user_stack[stack_words - 1]
        *--user_esp = (uint32_t)arg;          // sera a esp + 4
        // new_processus->user_stack[stack_words - 2]
        *--user_esp = 0x01000005;             // sera a esp + 0       // adresse de retour du wrapper exit dans user/crt0.S

        uint32_t esp_user = (uint32_t)user_esp;

        // Calcul de l'adresse de la pile utilisateur
        new_processus->kernel_stack[MAX_STACK_SIZE - 1] = (uint32_t)USER_DS;    // SS_user
        new_processus->kernel_stack[MAX_STACK_SIZE - 2] = esp_user;             // sommet de la pile utilisateur
        new_processus->kernel_stack[MAX_STACK_SIZE - 3] = 0x202;                // EFLAGS : IF=1, IOPL=0
        new_processus->kernel_stack[MAX_STACK_SIZE - 4] = (uint32_t)USER_CS;    // CS_user
        new_processus->kernel_stack[MAX_STACK_SIZE - 5] = (uint32_t)pt_func;    // EIP_user (adresse d'entrée du code user)
        new_processus->kernel_stack[MAX_STACK_SIZE - 6] = (uint32_t)return_to_user; // adresse de retour pour ctx_sw

        // pointeur vers le wrapper return_to_user (iret)
        new_processus->registers[1] = (uint32_t)&new_processus->kernel_stack[MAX_STACK_SIZE - 6];

    } else {
        // Initialisation du processus kernel
        new_processus->is_user = false;
        new_processus->user_stack_size = 0;
        new_processus->user_stack = NULL;

        // Placer l'adresse de code en sommet de pile et initialiser %esp
        new_processus->kernel_stack[MAX_STACK_SIZE - 4] = (uint32_t)run_process_exec;
        new_processus->kernel_stack[MAX_STACK_SIZE - 2] = (uint32_t)pt_func;
        new_processus->kernel_stack[MAX_STACK_SIZE - 1] = (uint32_t)arg;

        new_processus->registers[1] = (uint32_t)&new_processus->kernel_stack[MAX_STACK_SIZE - 4]; // %esp -> wrapper d'exécution
        new_processus->registers[2] = (uint32_t)&new_processus->kernel_stack[MAX_STACK_SIZE - 3]; // %ebp -> valeur de retour du wrapper
    }

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
* Permet au processus actif de se mettre en sommeil jusqu'à un certains nombre de ticks depuis le démarrage du noyau.
*
* uint32_t date_tick : nombre de secondes pendant lesquelles le processus doit être endormi
* return : ne retourne rien, le processus se met en sommeil et rend la main à l'ordonnanceur
*/
void wait_clock(uint32_t date_tick) {
    if (!actif) {
        return; // Pas de processus actif
    }
    if(date_tick < current_clock()) {
        return; // Date de réveil déjà passée
    }
    actif->state = ENDORMI;
    actif->time_to_wake = -(date_tick); // temps de réveil inversé (tri croissant)

    ordonnance(); // Appeler l'ordonnanceur pour switcher vers un autre processus pendant que celui-ci est endormi
}
