#include "shell.h"
#include "func_test.h"
#include "stdio.h"
#include "syscall.h"
#include "string.h"
#include "stdint.h"

/**
 * Supprime les espaces au début d'une chaine s
 */
static char *skip_spaces(char *s)
{
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    return s;
}

/**
 * Fonction power équivalente à celle de math.h
 * 
 * int fact : facteur dont on veut la puissance
 * int pow : puissance voulue 
 * return : fact^pow
 */
uint32_t power(int fact, int pow) {
    uint32_t result = 1;

    if (pow < 0) {
        return 0;
    }

    for (int i = 0; i < pow; i++) {
        result *= (uint32_t)fact;
    }

    return result;
}

/**
 * Fonction atoi de C, converti str en entier
 * 
 */
int atoi(char* str) {
    int result = 0;
    int sign = 1;

    str = skip_spaces(str);

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

/**
 * Converti une chaine de caratère en identifiant
 */
static command_id_t command_id(const char *command)
{
    if (command[0] == '\0') {
        return CMD_EMPTY;
    }
    if (!strcmp(command, "sys_info") || !strcmp(command, "si")) {
        return CMD_SYS_INFO;
    }
    if (!strcmp(command, "ps")) {
        return CMD_PS;
    }
    if (!strcmp(command, "exit")) {
        return CMD_EXIT;
    }
    if (!strcmp(command, "echo") || !strncmp(command, "echo ", 5)) {
        return CMD_ECHO;
    }
    if (!strcmp(command, "help")) {
        return CMD_HELP;
    }
    if (!strcmp(command, "clear")) {
        return CMD_CLEAR;
    }
    if (!strcmp(command, "run_tests")) {
        return CMD_RUN_TESTS;
    }
    if (!strcmp(command, "test_run") || !strncmp(command, "test_run ", 9)) {
        return CMD_TEST_N;
    }
    if (!strcmp(command, "test_sys_info")) {
        return CMD_TEST_SYS_INFO;
    }
    if (!strcmp(command, "colors") || !strncmp(command, "colors ", 7)) {
        return CMD_COLOR;
    }
    return CMD_UNKNOWN;
}

/**
 * Affiche les commandes disponibles et leur fonction
 */
static void print_help(void)
{
    printf("Commandes disponibles:\n");
    printf("  help                  affiche cette aide\n");
    printf("  sys_info | si         affiche les infos systeme\n");
    printf("  ps                    affiche la liste des processus\n");
    printf("  echo on|off           active/desactive l'echo clavier\n");
    printf("  exit                  quitte le shell\n");
    printf("  clear                 nettoie la console\n");
    printf("  colors CT CF          change les couleurs\n");
    printf("  colors --help         affiche les couleurs disponibles\n");
    printf("  colors --reset        remet les couleurs par defaut\n");
    printf("  run_tests             lance la totalite des tests utilisateur\n");
    printf("  test_run N            lance le test N\n");
    printf("  test_sys_info         teste l'affichage sys_info\n");
}

/**
 * Affiche l'aide de la commande colors
 */
void print_colors_help(void) {
    printf("usage:\n");
    printf("  colors CT CF\n");
    printf("  colors --reset\n");
    printf("  colors --help\n");
    printf("\n");
    printf("CT = couleur de fond, 0-7\n");
    printf("CF = couleur du texte, 0-15\n");
    printf("\n");
    printf("Change les couleurs (fond 0-7, texte 0-15) en suivant le tableau suivant :\n");
    printf("+-----+---------+-----+----------+-----+-------------+-----+----------------+\n");
    printf("| val | couleur | val | couleur  | val | couleur     | val | couleur        |\n");
    printf("+=====+=========+=====+==========+=====+=============+=====+================+\n");
    printf("| 0   | noir    | 4   | rouge    | 8   | gris fonce  | 12  | rouge clair    |\n");
    printf("+-----+---------+-----+----------+-----+-------------+-----+----------------+\n");
    printf("| 1   | bleu    | 5   | magenta  | 9   | bleu clair  | 13  | magenta clair  |\n");
    printf("+-----+---------+-----+----------+-----+-------------+-----+----------------+\n");
    printf("| 2   | vert    | 6   | marron   | 10  | vert clair  | 14  | jaune          |\n");
    printf("+-----+---------+-----+----------+-----+-------------+-----+----------------+\n");
    printf("| 3   | cyan    | 7   | gris     | 11  | cyan clair  | 15  | blanc          |\n");
    printf("+-----+---------+-----+----------+-----+-------------+-----+----------------+\n");

}

/**
 * Gère les différentes options de la commande 'colors'
 * 
 * char* command : 'colors' avec ses arguments
 * return : 0 si tout va bien, -1 sinon
 */
int command_colors(char* command) {
    char *arg = skip_spaces(command + 6);

    if (!strcmp(arg, "--help")) {
        print_colors_help();
        return 0;
    }

    if (!strcmp(arg, "--reset")) {
        change_colors(0, 15);
        printf("colors reset\n");
        return 0;
    }

    int ct = atoi(arg);

    while (*arg && *arg != ' ' && *arg != '\t') {
        arg++;
    }
    arg = skip_spaces(arg);

    if (*arg == '\0') {
        printf("usage: colors CT CF\n");
        return -1;
    }

    int cf = atoi(arg);

    if (ct < 0 || ct > 7 || cf < 0 || cf > 15) {
        printf("usage: colors CT CF (CT 0-7, CF 0-15)\n");
        return -1;
    }

    change_colors(ct, cf);
    return 0;
}

int shell(void* arg) {
    char buffer[100];

    printf("Lancement de super-shell!\n");
    (void) arg;

    while(1) {
        printf("super-shell $ ");
        // TODO: empecher l'utilisateur de supprimer le "super-shell $ "
        read_line(buffer, 100);
        exec_command(buffer);
    }

    return 0;
}

/**
 * Reads a line of input from the console into the provided buffer.
 * The function reads characters until a newline or carriage return is encountered,
 * or until the maximum length is reached.
 *
 * char* buffer : the buffer to store the input line.
 * int max_length : the maximum number of characters to read
 * return : the number of characters read
 */
int read_line(char* buffer, int max_length) {
    char c;
    int i = 0;

    while(i < max_length - 1) {
        if (!cons_read(1, &c)) { continue; }
        if (c == '\b' || c == 127) {
            if (i == 0) { continue; }
            i--;
            buffer[i] = '\0';
            continue;
        }
        if (c == '\n' || c == '\r') {
            buffer[i] = '\0';
            break;
        }
        buffer[i++] = c;
    }

    return i;
}

/**
 * Execute la commande "command" via un appel système
 */
int exec_command(char* command) {
    command = skip_spaces(command);

    switch (command_id(command)) {
        case CMD_EMPTY:
            return 0;
        case CMD_SYS_INFO:
            sys_info();
            return 0;
        case CMD_PS:
            ps();
            return 0;
        case CMD_EXIT:
            printf("Bye.\n");
            exit(0);
        case CMD_ECHO: {
            char *arg = skip_spaces(command + 4);
            if (!strcmp(arg, "on")) {
                cons_echo(1);
                printf("echo on\n");
                return 0;
            }
            if (!strcmp(arg, "off")) {
                cons_echo(0);
                printf("echo off\n");
                return 0;
            }
            printf("usage: echo on|off\n");
            return -1;
        }
        case CMD_HELP:
            print_help();
            return 0;
        case CMD_CLEAR:
            printf("\f");
            return 0;
        case CMD_RUN_TESTS:
            int res = run_test_proc(20);
            if(!res) {
                printf("TESTS SUCCEEDED!\n");
            }
            return 0;
        case CMD_TEST_N:
            char *arg = skip_spaces(command + 8);
            int number = atoi(arg);
            test_run(number);
            return 0;
        case CMD_TEST_SYS_INFO:
            return test_sys_info();
        case CMD_COLOR: 
            return command_colors(command);
        case CMD_UNKNOWN:
        default:
            printf("commande inconnue: %s\nEssaie la commande 'help'\n", command);
            return -1;
    }
}
