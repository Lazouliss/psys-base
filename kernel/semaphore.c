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
    (void)sem;
    return 0;
}

int try_wait(int sem) {
    (void)sem;
    return 0;
}

int scount(int sem) {
    (void)sem;
    return 0;
}

int sreset(int sem, short int count) {
    (void)sem;
    (void)count;
    return 0;
}
