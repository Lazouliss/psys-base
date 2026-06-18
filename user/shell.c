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
    printf("  run_tests             lance la totalite des tests utilisateur\n");
    printf("  test_run N            lance le test N\n");
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
        case CMD_UNKNOWN:
        default:
            printf("commande inconnue: %s\n", command);
            return -1;
    }
}
