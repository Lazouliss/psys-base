#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "stdint.h"
#include "debug.h"
#include "queue.h"

#define NBQUEUE 100
#define MAX_BUFFER 992

typedef struct message
{
    uint32_t fid;

    int *buffer;                        // tampon circulaire pre-alloue
    uint32_t buf_head;                  // indice de lecture
    uint32_t buf_tail;                  // indice d'ecriture
    uint32_t count;                     // nombre de messages actuels
    uint32_t capacity;                  // capacite max

    link sender_queue;                  // queue des processus bloqués à l'envoi un message
    link receiver_queue;                // queue des processus bloqués au moment de recevoir un message

} message_t;

extern message_t* message_tab[NBQUEUE];

int pcreate(int count);
int psend(int fid, int message);
int preceive(int fid,int *message);
int pcount(int fid, int count[static 1]);
int pdelete(int fid);
int preset(int fid);

#endif
