#ifndef __PROCESSUS_H__
#define __PROCESSUS_H__

#include "stdint.h"
#include "queue.h"
#include "linked_list.h"

#define MAX_STACK_SIZE 2048
#define NBPROC 30
#define DEFAULT_PRIO 128
#define MAX_PRIO 255

typedef enum
{
    ACTIVABLE = 0,      // Le processus n'attend que la possession du processeur pour s'exécuter.
    ELU = 1,            // Le processus est celui qui possède le processeur.
    ENDORMI = 2,        // Le processus a appelé wait_clock, la primitive de mise en sommeil jusqu'à une heure donnée.
    BLOCK_MSG_RCV = 3,  // Le processus a exécuté une opération sur une file de message qui attend un message pour progresser (par exemple preceive).
    BLOCK_MSG_SND = 4,  // Le processus a exécuté une opération sur une file de message qui attend d'envoyer un message pour progresser (par exemple psend).
    BLOCK_SEM = 5,      // Le processus a exécuté une opération sur un sémaphore qui demande d'attendre pour progresser (par exemple wait).
    BLOCK_IO = 6,       // Le processus attend qu'une entrée/sortie soit réalisée.
    BLOCK_CHILD = 7,    // Le processus attend qu'un de ses processus fils soit terminé.
    ZOMBIE = 8,         // Le processus a terminé son exécution ou a été terminé par l'appel système kill et son père est toujours vivant et n'a pas encore fait de waitpid sur lui.
    DYING = 9           // Le processus a terminé son exécution et il doit être nettoyé
} states;

typedef struct processus
{
    uint32_t pid;
    uint32_t p_pid;
    int32_t blocking_cid;           // le pid de l'enfant ou -1 pour n'importe lequel
    int retval;

    int32_t blocking_fid;           // le fid de la file de message sur laquelle il est bloqué ou -1 si le processus était bloqué et que fid a été pdelete
    int message;                    // message reçu par le processus

    const char* name;
    states state;                   // Process state (0: ready / activable, 1: running / élu, 2: sleeping / endormi)
    uint32_t registers[5];          // CPU registers (ebx, esp, ebp, esi, edi)
    uint32_t stack[MAX_STACK_SIZE]; // pile d'execution des processus
    int32_t prio;                   // priorité du processus (pour l'ordonnanceur)
    int32_t time_to_wake;           // nombre de secondes avant de se réveiller (pour les processus endormis)

    link link;                      // pointeur vers le processus suivant dans la liste des processus
    struct processus* children;     // liste des processus fils (simple linked list)
    simple_link siblings;           // pointeur vers le processus frere suivant
} processus_t;

extern link queue_process;
extern link queue_process_sleeping;
extern link queue_process_zombie;
extern link queue_process_blocked;

extern processus_t* processus_tab[NBPROC];

extern processus_t* actif;
extern void ctx_sw(uint32_t* old_reg, uint32_t* new_reg); // Fonction context_switch en assembleur (ctx_sw.S)

int32_t getpid();
const char* mon_nom();
int getprio(int pid);

int chprio(int pid, int newprio);

void ordonnance(void);

int start(int (*pt_func)(void*), [[maybe_unused]] unsigned long ssize_user, int prio, const char* name, void *arg);
void wait_clock(uint32_t nbr_secs);
int kill(int pid);
__attribute__((noreturn))
void exit(int retval);
int waitpid(int pid, int *retvalp);

// Fonctions de debug
void print_queues();
void print_queue();
void print_queue_sleeping();
void print_queue_zombie();
void print_queue_blocked();

#endif
