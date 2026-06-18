#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "stdint.h"
#include "stdbool.h"
#include "cpu.h"
#include "string.h"

#define VGABASE ((uint16_t*)0xB8000)
#define ROW_MAX 25
#define COL_MAX 80
#define DEFAULT_BG_COLOR 0          // ct : couleur de fond
#define DEFAULT_FG_COLOR 15         // cf : couleur du texte

const uint16_t* ptr_mem(uint32_t lig, uint32_t col);
void change_colors(uint8_t ct, uint8_t cf);
void ecrit_car(uint32_t lig, uint32_t col, char c, uint8_t ct, uint8_t cf, bool cl);
void place_curseur(uint32_t lig, uint32_t col);
void efface_ecran(void);
void place_next_pos_cursor();
void traite_car(char c);
void defilement();

#endif
