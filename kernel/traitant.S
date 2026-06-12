# cette directive sert a rendre l'etiquette publique
    .globl traitant_IT_32
# debut du traitant 
traitant_IT_32:
# sauvegarde des registres importants
    pushl %eax
    pushl %edx
    pushl %ecx
# appel a la fonction C realisant le traitant
    call tic_PIT
# restauration des registres importants
    popl %ecx
    popl %edx
    popl %eax
# fin du traitant
    iret

# Copié collé de l'ass précédent
    .globl traitant_IT_49
# debut du traitant
traitant_IT_49:
# sauvegarde des registres importants
    pushl %eax
    pushl %edx
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %esp
# appel a la fonction C realisant le traitant
    call syscall_PIT
# restauration des registres importants
    popl %esp
    popl %ebx
    popl %edx
    popl %ecx
    popl %edx
    popl %eax
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
    iret
