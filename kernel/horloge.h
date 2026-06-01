#include "screen.h"
#include "debug.h"
#include "../shared/stdio.h"

#define QUARTZ 0x1234DD
#define CLOCKFREQ 50

void print_horloge(char* time_str);
extern void traitant_IT_32(void);
void tic_PIT(void);
void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
