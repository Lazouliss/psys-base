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
    if (!strcmp(command, "test")) {
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
    printf("  help             affiche cette aide\n");
    printf("  sys_info | si    affiche les infos systeme\n");
    printf("  ps               affiche la liste des processus\n");
    printf("  echo on|off      active/desactive l'echo clavier\n");
    printf("  exit             quitte le shell\n");
    printf("  clear            nettoie la console\n");
    printf("  run_tests        lance la totalite des tests utilisateur\n");
    printf("  test N           lance le test N\n");
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
            printf("TODO\n");
            return 0;
        case CMD_UNKNOWN:
        default:
            printf("commande inconnue: %s\n", command);
            return -1;
    }
}
