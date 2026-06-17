# cette directive sert a rendre l'etiquette publique
    .globl traitant_IT_32
# debut du traitant 
traitant_IT_32:
# sauvegarde des registres importants
    pushl %eax
    pushl %edx
    pushl %ecx
    pushl %ds
    pushl %es
    movw $0x18, %ax
    movw %ax, %ds
    movw %ax, %es
# appel a la fonction C realisant le traitant
    call tic_PIT
# restauration des registres importants
    popl %es
    popl %ds
    popl %ecx
    popl %edx
    popl %eax
# fin du traitant
    iret

# Gestion des interruptions user avec le traitant 49
# Récupère les arguments passés dans la stack, puis exécute la fonction appelée
# int32_t syscall_PIT(int32_t num, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5);
# eax : num -> identifiant de la fonction
# ebx : a1
# ecx : a2
# edx : a3
# esi : a4
# edi : a5
    .globl traitant_IT_49
# debut du traitant
traitant_IT_49:
# sauvegarde des registres importants
    pushl %ds
    pushl %es
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    movw $0x18, %ax
    movw %ax, %ds
    movw %ax, %es
# appel a la fonction C realisant le traitant
    call syscall_PIT
# restauration des registres importants
    addl $4, %esp
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %es
    popl %ds
# fin du traitant
    iret

# Fonction appelé avant le ct_sw, et contient la stack suivante :
#   [esp+0]  = EIP_user    (adresse d'entrée du code user)
#   [esp+4]  = CS_user     (0x43)
#   [esp+8]  = EFLAGS      (0x202 : IF=1, IOPL=0)
#   [esp+12] = ESP_user    (sommet de la pile utilisateur)
#   [esp+16] = SS_user     (0x4b)

    .globl return_to_user
return_to_user:
    movw $0x4b, %ax
    movw %ax, %ds
    movw %ax, %es
    iret


    .globl traitant_IT_33
traitant_IT_33:
    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ds
    pushl %es

    movw $0x18, %ax
    movw %ax, %ds
    movw %ax, %es

    call keyboard_PIT

    popl %es
    popl %ds
    popl %edx
    popl %ecx
    popl %eax

    iret
