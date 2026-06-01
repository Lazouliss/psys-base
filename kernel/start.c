#include "debugger.h"
#include "cpu.h"
#include "../shared/stdio.h"
#include "../kernel/screen.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void)
{
	int i;
	// call_debugger(); useless with qemu -s -S
	i = 10;
	i = fact(i);


	/************************/
	/* Tests simples printf */
	/************************/
	//efface_ecran();
	// Supprime le contenu de l'écran de base
	printf("\f");
	// Affiche un simple texte qui disparaitra
	printf("Hello, World!\n");
	// Affiche le résultat contenu dans la variable i
	printf("10! = %d\n", i);
	// Test tabulation
	printf("tab\tulation\n");
	// Test defilement()
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nderniere lignes");
	// Test place_curseur() au milieu de l'écran
	place_curseur(12, 40);
	// Reviens au début de la ligne, puis écrit 5 caractères, et place le curseur SUR le dernier caractère
	printf("\r klfd\b");
	
	while(1)
	  hlt();

	return;
}
