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
	efface_ecran();
	printf("Hello, World!\n");
	printf("10! = %d\n", i);
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nderniere lignes");
	
	while(1)
	  hlt();

	return;
}
