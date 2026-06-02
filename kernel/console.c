#include "screen.h"
#include "console.h"

uint16_t ROW = 0;
uint16_t COL = 0;

const uint16_t* ptr_mem(uint32_t lig, uint32_t col)
{
    return VGABASE + (lig * COL_MAX + col);
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
    *ptr = (cl << 15) | (cf << 12) | (ct << 8) | c; 
}

void place_curseur(uint32_t lig, uint32_t col)
{
    ROW = lig;
    COL = col;
    uint16_t pos = ROW * COL_MAX + COL;
      
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
    for (uint32_t lig = 0; lig < ROW_MAX; lig++) {
        for (uint32_t col = 0; col < COL_MAX; col++) {
            // Efface l'écran en écrivant un espace avec les couleurs par défaut (caractère blanc sur fond noir)
            ecrit_car(lig, col, ' ', 15, 0, false);
        }
    }
    ROW = 0;
    COL = 0;
    place_curseur(ROW, COL);
}

/* Fonction qui calcule et positionne le curseur aux coordonnées +1, bornées dans l'écran */
void place_next_pos_cursor(){
    if(++COL >= COL_MAX) {
        COL = 0;
        ROW++; // Dépassement de la dernière ligne, prit en compte par défillement() dans console_putbytes()
    }
    place_curseur(ROW, COL);
}

void traite_car(char c)
{
    // Switch pour les caractères, suivi du placement du curseur
    switch (c)
    {
    case 8: // Backspace
        ecrit_car(ROW, COL, ' ', 15, 0, false);
        if(COL > 0) {
            COL--;
        } else if(ROW > 0) {
            ROW--;
            COL = COL_MAX - 1;
        }
        break;
    case 9: // Tab
        COL = (COL + 8) & ~(8 - 1); // Avance de 8 positions, aligné sur une tabulation
        if(COL >= COL_MAX) {
            COL = 0;
            ROW++;
        }
        break;
    case 10: // Newline
        COL = 0;
        ROW++; // Dépassement de la dernière ligne, prit en compte par défillement() dans console_putbytes()
        break;
    case 12: // efface l'écran
        efface_ecran();
        break;
    case 13: // Retour chariot
        COL = 0;
        break;
    case 32 ... 126:
        // Comportement par défaut : Afficher le caractère à la position actuelle du curseur
        ecrit_car(ROW, COL, c, 15, 0, false); // Affiche le caractère avec les couleurs par défaut
        place_next_pos_cursor();
        break;
    default:
        // Caractères non imprimables autres que ceux gérés sont ignorés
    }
    place_curseur(ROW, COL);
}

void defilement()
{
    // Remonte les lignes de 1 à ROM_MAX, de une ligne
    memmove((void*)ptr_mem(0, 0), (void*)ptr_mem(1, 0), (ROW_MAX - 1) * COL_MAX * sizeof(uint16_t));
    // Efface la dernière ligne après le défilement
    for (uint32_t col = 0; col < COL_MAX; col++) {
        ecrit_car(ROW_MAX - 1, col, ' ', 15, 0, false); // Efface la dernière ligne
    }
}

void console_putbytes(const char *chaine, int taille)
{
    for (int i = 0; i < taille; i++) {
        traite_car(chaine[i]);
        if(ROW >= ROW_MAX) {
            defilement();
            ROW = ROW_MAX - 1;
        }
    }
}
