#include "horloge.h"
#include "segment.h"

uint32_t ticks = 0;

void print_horloge(char* time_str)
{
    place_curseur(0, COL_MAX - 8); // Place le curseur à la ligne 0, colonne 72 (8 caractères pour l'heure)
    printf("%s", time_str);
}

void tic_PIT(void) 
{
    outb(0x20, 0x20);

    // increment ticks
    ticks++;

    // Mise à jour à chaque seconde
    if (ticks % CLOCKFREQ == 0) {
        uint32_t total_seconds = ticks / CLOCKFREQ;
        uint32_t hours = (total_seconds / 3600) % 24;
        uint32_t minutes = (total_seconds / 60) % 60;
        uint32_t seconds = total_seconds % 60;
        
        char time_str[9];
        sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
        print_horloge(time_str);
    }
}

void init_traitant_IT(int32_t num_IT, void (*traitant)(void))
{
    uint32_t addr = (uint32_t)traitant;
    // Premier mot : KERNEL_CS en bits 31-16, bits bas de l'adresse en bits 15-0
    IDT_BASE[num_IT * 2]     = ((uint32_t)KERNEL_CS << 16) | (addr & 0xFFFF);
    // Deuxième mot : bits hauts de l'adresse en bits 31-16, 0x8E00 en bits 15-0
    IDT_BASE[num_IT * 2 + 1] = (addr & 0xFFFF0000) | 0x8E00;
}
