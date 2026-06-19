# Projet Système

## Description du projet

L'objectif du projet système était la réalisation d'un noyau de système d'exploitation sur une architecture Intel x86 et, le risc-v 64 bits. En pratique, en partant de rien (ou presque), nous avons mis en œuvre un certain nombre de concepts clés associés aux systèmes d'exploitation, comme, par exemple :

- la création et l'exécution des processus ;
- leur synchronisation ;
- leur ordonnancement ;
- la gestion des entrées/sorties (clavier, écran) ;
- l'implémentation d'un interprète de commandes.

Ce projet a été réalisé en fin de deuxième année à l'ENSIMAG. Il était très intéressant et challengeant.

### Lien du sujet complet

https://systemes.pages.ensimag.fr/psys-doc/

## Lancer le projet

Ce projet fonctionne avec des émulateurs comme [Qemu](https://systemes.pages.ensimag.fr/psys-doc/Qemu/) ou [Bochs](https://systemes.pages.ensimag.fr/psys-doc/Bochs/).

On peut les lancer avec les commandes suivantes :

```bash
make qemu
make bochs
```

### Lancer le débuggeur dans VSCode

Commencer par ajouter des breakpoints aux endroits souhaités dans le code.

```bash
make qemu-gdb
# puis dans VSCode, dans le menu "Run and Debug", cliquer sur "Start"
```
