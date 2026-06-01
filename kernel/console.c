#include "stdint.h"
#include "stdbool.h"
#include "cpu.h"

const uint16_t* VGABASE = (uint16_t*)0xB8000;

const uint16_t* ptr_mem(uint32_t lig, uint32_t col)
{
    return VGABASE + (lig * 80 + col);
}

/*
* Paramètres :
* c  : caractère à afficher
* ct : couleur de fond
* cf : couleur de caractère
* cl : clignotement
*/
void ecrit_car(uint32_t lig, uint32_t col, char c, uint8_t ct, uint8_t cf, bool cl)
{
    uint16_t* ptr = (uint16_t*)ptr_mem(lig, col);
    *ptr = (c << 8) | (cl << 7) | (cf << 4) | (ct); 
}

void place_curseur(uint32_t lig, uint32_t col)
{
    uint16_t pos = lig * 80 + col;
      
    // indiquer à la carte que l'on va envoyer la partie basse de la position du curseur
    outb(0x0F, 0x3D4);
    // envoyer cette partie basse sur le port de données
    outb(pos & 0xFF, 0x3D5);
    // signaler qu'on envoie maintenant la partie haute
    outb(0x0E, 0x3D4);
    // envoyer la partie haute de la position sur le port de données
    outb((pos >> 8) & 0xFF, 0x3D5);
}

void efface_ecran(void)
{
    for (uint32_t lig = 0; lig < 25; lig++) {
        for (uint32_t col = 0; col < 80; col++) {
            // Efface l'écran en écrivant un espace avec les couleurs par défaut (caractère blanc sur fond noir)
            ecrit_car(lig, col, ' ', 15, 0, false);
        }
    }
    place_curseur(0, 0);
}
