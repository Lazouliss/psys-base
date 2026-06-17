#ifndef __SHELL_H__
#define __SHELL_H__

typedef enum {
    CMD_EMPTY,
    CMD_SYS_INFO,
    CMD_PS,
    CMD_EXIT,
    CMD_ECHO,
    CMD_HELP,
    CMD_UNKNOWN
} command_id_t;

int shell(void* arg);
int read_line(char* buffer, int max_length);
int exec_command(char* command);

#endif
