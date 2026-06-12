#include "../shared/syscall_numbers.h"

# Signature des fonctions côté User
# Chaque fonction sauvegarde les paramètres dans la stack
# Le kernel les récupères ensuite

    .text
    .globl cons_write
cons_write:
    movl $SYS_CONS_WRITE, %eax
    movl 4(%esp), %ebx
    movl 8(%esp), %ecx
    int $49
    ret
