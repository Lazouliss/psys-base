#include "horloge.h"

void print_horloge(char* time_str)
{
    place_curseur(0, COL_MAX - 8); // Place le curseur à la ligne 0, colonne 72 (8 caractères pour l'heure)
    printf("%s", time_str);
}
