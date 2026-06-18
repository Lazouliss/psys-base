#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "stdint.h"
#include "stdbool.h"
#include "queue.h"
#include "processus.h"

#define NBSEMS 100

typedef struct semaphore {
    bool valid;
    int16_t value;
    link wait_queue;
} semaphore_t;

extern semaphore_t* semaphore_tab[NBSEMS];

int screate(short int count);
int sdelete(int sem);
int signal(int sem);
int signaln(int sem, short int count);
int wait(int sem);
int try_wait(int sem);
int scount(int sem);
int sreset(int sem, short int count);

#endif
