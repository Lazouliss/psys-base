#include "horloge.h"
#include "segment.h"
#include "screen.h"
#include "processus.h"

uint32_t ticks = 0;    // to test : 183550 => 01:01:11

void print_horloge(char* time_str)
{
    // Calcul de la longueur de la chaîne (en général 8 pour "HH:MM:SS")
    int len = 0;
    while (time_str[len])
        len++;

    int col_start = COL_MAX - len;
    for (int i = 0; i < len; i++) {
        // noir sur fond gris, sans clignotement -> pour mettre l'horloge en évidence
        ecrit_car(0, col_start + i, time_str[i], 0, 7, false);
    }
}

void tic_PIT(void) 
{
    outb(0x20, 0x20);

    // increment ticks
    ticks++;

    // Changement de contexte
    if (ticks % SCHEDFREQ == 0) {
        ordonnance();
    }

    // Mise à jour de l'horloge à chaque seconde
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

// Masque (masque=true) ou démasque (masque=false) l'IRQ num_IRQ (0-7)
void masque_IRQ(uint32_t num_IRQ, bool masque)
{
    // lire la valeur actuelle du masque sur le port de données 0x21
    uint8_t mask = inb(0x21);
    // IRQ N vaut 1 si IRQ est masquée, 0 si elle est autorisée
    if (masque)
        mask |= (uint8_t)(1u << num_IRQ);
    else
        mask &= (uint8_t)(0u << num_IRQ);      // (uint8_t)~(1u << num_IRQ);
    // envoyer le masque sur le port de données 0x21
    outb(mask, 0x21);
}

// Configure la fréquence de l'horloge programmable et installe le traitant
void config_horloge(void)
{
    // installation du traitant pour l'interruption 32 (IRQ0)
    init_traitant_IT(32, traitant_IT_32);

    // on envoie la commande sur 8 bits 0x34 sur le port de commande 0x43
    outb(0x34, 0x43);
    // 8 bits de poids faibles
    outb((uint8_t)((QUARTZ / CLOCKFREQ) % 256), 0x40);
    // 8 bits de poids forts
    outb((uint8_t)((QUARTZ / CLOCKFREQ) / 256), 0x40);

    // démasquage de l'IRQ0 pour autoriser les signaux de l'horloge
    masque_IRQ(0, false);
}

int32_t nbr_secondes(void) {
    return ticks / CLOCKFREQ;
}
