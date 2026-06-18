#ifndef __SHELL_H__
#define __SHELL_H__

typedef enum {
    CMD_EMPTY,
    CMD_SYS_INFO,
    CMD_PS,
    CMD_EXIT,
    CMD_ECHO,
    CMD_HELP,
    CMD_CLEAR,
    CMD_RUN_TESTS,
    CMD_TEST_N,
    CMD_TEST_SYS_INFO,
    CMD_COLOR,
    CMD_UNKNOWN
} command_id_t;

int shell(void* arg);
int read_line(char* buffer, int max_length);
int exec_command(char* command);

#endif
