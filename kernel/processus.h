#ifndef __PROCESSUS_H__
#define __PROCESSUS_H__

#include "stdint.h"
#include "queue.h"

#define MAX_STACK_SIZE 512
#define MAX_PROCESSES 8

typedef enum
{
    ACTIVABLE = 0,
    ELU = 1,
    ENDORMI = 2
} states;

typedef struct
{   
    uint32_t pid;
    char* name;
    states state;                   // Process state (0: ready / activable, 1: running / élu, 2: sleeping / endormi)
    uint32_t registers[5];          // CPU registers (ebx, esp, ebp, esi, edi)
    uint32_t stack[MAX_STACK_SIZE]; // pile d'execution des processus
    int32_t prio;                   // priorité du processus (pour l'ordonnanceur)
    link link;                      // pointeur vers le processus suivant dans la liste des processus
    int32_t time_to_wake;          // nombre de secondes avant de se réveiller (pour les processus endormis)
} processus_t;

extern link queue_process;
extern link queue_process_sleeping;

extern processus_t* actif;
extern void ctx_sw(uint32_t* old_reg, uint32_t* new_reg); // Fonction context_switch en assembleur (ctx_sw.S)

int32_t mon_pid();
char* mon_nom();
void ordonnance(void);

int32_t cree_processus(void (*code)(void), char *nom);
void dors(uint32_t nbr_secs);

// Fonctions de debug
void print_queue();
void print_queue_sleeping();

#endif
