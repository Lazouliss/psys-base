#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "stdint.h"
#include "stdbool.h"

extern void traitant_IT_33(void);
void init_keyboard(void);

void keyboard_interrupt(void);
void keyboard_data(char *str);
void kbd_leds(unsigned char leds);

bool keyboard_has_char(void);
char keyboard_pop_char(void);

void cons_echo(int on);
int cons_read(unsigned long size, char str[static size]);

#endif
