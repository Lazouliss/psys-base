#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "stdint.h"
#include "debug.h"
#include "queue.h"
#include "linked_list.h"

#define NBQUEUE 10

typedef struct msg_node {
    int value;
    simple_link link;
} msg_node_t;

typedef struct message
{
    uint32_t fid;

    msg_node_t *msg_file;               // file de message
    uint32_t size_msg_file;             // current size
    uint32_t max_size_msg_file;         // max size

    link sender_queue;                  // queue des processus bloqués à l'envoi un message
    link receiver_queue;                // queue des processus bloqués au moment de recevoir un message

} message_t;

extern uint32_t last_queue;
extern message_t* message_tab[NBQUEUE];

int pcreate(int count);
int psend(int fid, int message);
int preceive(int fid,int *message);
int pcount(int fid, int count[static 1]);
int pdelete(int fid);
int preset(int fid);

#endif
