#ifndef __HORLOGE_H__
#define __HORLOGE_H__

#include "stdint.h"
#include "stdbool.h"
#include "cpu.h"
#include "stdint.h"
#include "stdbool.h"
#include "debug.h"

#define QUARTZ 0x1234DD
#define CLOCKFREQ 50
#define IDT_BASE ((uint32_t *)0x1000)

// fonction assembleur dans traitant.S
extern void traitant_IT_32(void);

void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
void masque_IRQ(uint32_t num_IRQ, bool masque);
void config_horloge(void);
void tic_PIT(void);

#endif
