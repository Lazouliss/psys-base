#ifndef __HORLOGE_H__
#define __HORLOGE_H__

#include "stdint.h"
#include "stdbool.h"
#include "cpu.h"
#include "stdint.h"
#include "stdbool.h"
#include "debug.h"

#define QUARTZ 0x1234DD
#define CLOCKFREQ 100
#define SCHEDFREQ 50
#define IDT_BASE ((uint32_t *)0x1000)

// fonction assembleur dans traitant.S
extern void traitant_IT_32(void);

void clock_settings(unsigned long quartz[static 1], unsigned long ticks[static 1]);

void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
void init_syscall(int32_t num_IT, void (*traitant)(void));
void masque_IRQ(uint32_t num_IRQ, bool masque);
void config_horloge(void);
void tic_PIT(void);
unsigned long current_clock(void);

#endif
