#include "keyboard.h"
#include "keyboard-glue.h"
#include "kbd.h"
#include "cpu.h"
#include "console.h"
#include "horloge.h"

#define KEYBOARD_BUFFER_SIZE 256

static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static unsigned read_pos = 0;
static unsigned write_pos = 0;
static bool echo_enabled = true;

void init_keyboard(void) {
	init_traitant_IT(33, traitant_IT_33);
	masque_IRQ(1, false);       // autorise les interruptions clavier
}

static bool buffer_empty(void)
{
    return read_pos == write_pos;
}

static bool buffer_full(void)
{
    return ((write_pos + 1) % KEYBOARD_BUFFER_SIZE) == read_pos;
}

static void buffer_push(char c)
{
    if (buffer_full()) {
        return;
    }

    keyboard_buffer[write_pos] = c;
    write_pos = (write_pos + 1) % KEYBOARD_BUFFER_SIZE;
}

bool keyboard_has_char(void)
{
    return !buffer_empty();
}

char keyboard_pop_char(void)
{
    if (buffer_empty()) {
        return 0;
    }

    char c = keyboard_buffer[read_pos];
    read_pos = (read_pos + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

void keyboard_PIT(void)
{
    unsigned char scancode;

    /* Lire le scancode clavier */
    scancode = inb(0x60);

    /* Acquitter IRQ1 sur le PIC maître */
    outb(0x20, 0x20);

    /* Traduire le scancode en caractères */
    do_scancode(scancode);
}

void keyboard_data(char *str)
{
    while (*str) {
        buffer_push(*str);

        if (echo_enabled) {
            cons_write(1, str);
        }

        str++;
    }

    /*
     * Plus tard : réveiller ici les processus bloqués dans cons_read.
     * Par exemple : passer leur état BLOCK_IO -> ACTIVABLE.
     */
}

void cons_echo(int on)
{
    echo_enabled = on != 0;
}

void kbd_leds(unsigned char leds)
{
    (void)leds;
}

int cons_read(unsigned long size, char str[static size])
{
    unsigned long i = 0;

    while (i < size && keyboard_has_char()) {
        str[i++] = keyboard_pop_char();
    }

    return i;
}
