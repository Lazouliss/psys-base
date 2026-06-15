#include "../shared/syscall_numbers.h"

# Wrappers d'appels systeme cote user.
# Convention:
#   eax = numero de syscall
#   ebx, ecx, edx, esi, edi = arguments 1 a 5
#   eax = valeur de retour apres int $49

# Définition des macros génériques
    .text

    .macro SYSCALL0 name, number
    .globl \name
\name:
    movl $\number, %eax
    int $49
    ret
    .endm

    .macro SYSCALL1 name, number
    .globl \name
\name:
    pushl %ebx

    movl $\number, %eax
    movl 8(%esp), %ebx
    int $49

    popl %ebx
    ret
    .endm

    .macro SYSCALL2 name, number
    .globl \name
\name:
    pushl %ecx
    pushl %ebx

    movl $\number, %eax
    movl 12(%esp), %ebx
    movl 16(%esp), %ecx
    int $49

    popl %ebx
    popl %ecx
    ret
    .endm

    .macro SYSCALL5 name, number
    .globl \name
\name:
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    movl $\number, %eax
    movl 24(%esp), %ebx
    movl 28(%esp), %ecx
    movl 32(%esp), %edx
    movl 36(%esp), %esi
    movl 40(%esp), %edi
    int $49

    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    ret
    .endm

# Définition et appel des fonctions
    .globl exit
exit:
    movl $SYS_EXIT, %eax
    movl 4(%esp), %ebx
    int $49
1:
    jmp 1b

    SYSCALL2 cons_write, SYS_CONS_WRITE
    SYSCALL0 getpid, SYS_GETPID
    SYSCALL1 getprio, SYS_GETPRIO
    SYSCALL1 kill, SYS_KILL
    SYSCALL2 pcount, SYS_PCOUNT
    SYSCALL1 pcreate, SYS_PCREATE
    SYSCALL1 pdelete, SYS_PDELETE
    SYSCALL2 preceive, SYS_PRECEIVE
    SYSCALL1 preset, SYS_PRESET
    SYSCALL2 psend, SYS_PSEND
    SYSCALL0 current_clock, SYS_CLOCK
    SYSCALL2 clock_settings, SYS_CLOCK_SETTINGS
    SYSCALL1 wait_clock, SYS_WAIT_CLOCK
    SYSCALL5 start, SYS_START
    SYSCALL2 waitpid, SYS_WAITPID
    SYSCALL2 place_curseur, SYS_PLC_CURS
    SYSCALL0 defilement, SYS_DEFILEMENT
