#include "semaphore.h"
#include "mem.h"
#include "stdbool.h"
#include "stdint.h"
#include "processus.h"

semaphore_t* semaphore_tab[NBSEMS];
int32_t last_idx = 0;

int screate(short int count) {
    if (count < 0) { return -1; }

    bool space_found = false;
    // Calcul du prochain PID disponible
    for (size_t i = 0; i < NBSEMS; i++)
    {
        if (semaphore_tab[(i+last_idx) % NBSEMS] == NULL || !semaphore_tab[i]->valid) {
            last_idx = (i+last_idx) % NBSEMS;
            space_found = true;
            break;
        }
    }
    if (!space_found) { return -1; }

    semaphore_t *s = mem_alloc(sizeof(semaphore_t));
    if (!s) { return -1; }

    INIT_LIST_HEAD(&s->wait_queue);
    s->valid = true;
    s->value = count;
    semaphore_tab[last_idx] = s;

    return last_idx;
}

int sdelete(int sem) {
    (void)sem;
    return 0;
}

int signal(int sem) {
    (void)sem;
    return 0;
}

int signaln(int sem, short int count) {
    (void)sem;
    (void)count;
    return 0;
}

int wait(int sem) {
    if (sem >= 0 && sem < NBSEMS && semaphore_tab[sem] && semaphore_tab[sem]->valid) { return -1; }
    semaphore_t *s = semaphore_tab[sem];
    // return -2 en cas de dépassement de capacité
    if (s->value == INT16_MIN) { return -2; }

    s->value--;
    if (s->value < 0) {
        actif->state = BLOCK_SEM;
        actif->blocking_fid = sem;
        actif->blocking_sem_ret = 0;
        queue_del(actif, link);
        queue_add(actif, &s->wait_queue, processus_t, link, prio);

        ordonnance();

        if (actif->blocking_sem_ret != 0) {
            return actif->blocking_sem_ret;
        }
    }
    return 0;
}

int try_wait(int sem) {
    (void)sem;
    return 0;
}

int scount(int sem) {
    if ((sem >= 0 && sem < NBSEMS) && semaphore_tab[sem] && semaphore_tab[sem]->valid) { return -1; }
    // les 16 bits de poids fort sont à 0, et les 16 bits de poids faible, interprétés comme un entier signé sur 16 bits, sont la valeur du sémaphore
    uint16_t value = semaphore_tab[sem]->value && 0x0000FFFF;
    return (int)value;
}

int sreset(int sem, short int count) {
    if ((sem >= 0 && sem < NBSEMS) && semaphore_tab[sem] && semaphore_tab[sem]->valid || count < 0) { return -1; }

    semaphore_t *s = semaphore_tab[sem];
    s->value = count;

    while (!queue_empty(&s->wait_queue)) {
        processus_t *proc = queue_out(&s->wait_queue, processus_t, link);
        proc->state = ACTIVABLE;
        proc->blocking_fid = -1;
        // pour le wait : -4 s'il est consécutif à sreset
        proc->blocking_sem_ret = -4;
        queue_add(proc, &queue_process, processus_t, link, prio);
    }

    ordonnance();

    return 0;
}
