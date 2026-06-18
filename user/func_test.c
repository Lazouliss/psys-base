#include "func_test.h"
#include "syscall.h"

static int sys_info_child(void *arg)
{
	int value = (int)arg;

	printf("[sys_info_child %d]\n", value);
	return value;
}

/**************************/
/* Décla function process */
/**************************/
int run_test_proc(int max_test) {
  int first_test = 1;

	for (int i = first_test; i <= max_test; i++) {
		printf("Test %d : ", i);
		test_run(i);
	}

  return 0;
}

int test1(int i) {
	printf("arg i : %d\n", i);
	return test_run(1);
}

int test_sys_info(void)
{
	int pid1;
	int pid2;
	int fid1;
	int fid2;

	fid1 = pcreate(4);
	fid2 = pcreate(2);
	if (fid1 < 0 || fid2 < 0) {
		printf("creation des files impossible\n");
		if (fid1 >= 0) {
			pdelete(fid1);
		}
		if (fid2 >= 0) {
			pdelete(fid2);
		}
		return -1;
	}

	psend(fid1, 42);
	psend(fid1, 43);

	pid1 = start(sys_info_child, 4000, 100, "info_child_1", (void *)1);
	pid2 = start(sys_info_child, 4000, 101, "info_child_2", (void *)2);
	if (pid1 < 0 || pid2 < 0) {
		printf("creation des processus impossible\n");
		if (pid1 > 0) {
			kill(pid1);
			waitpid(pid1, 0);
		}
		if (pid2 > 0) {
			kill(pid2);
			waitpid(pid2, 0);
		}
		pdelete(fid1);
		pdelete(fid2);
		return -1;
	}

	printf("=== sys_info apres creation ===\n");
	sys_info();

	kill(pid1);
	kill(pid2);
	waitpid(pid1, 0);
	waitpid(pid2, 0);
	pdelete(fid1);
	pdelete(fid2);

	printf("=== sys_info apres nettoyage ===\n");
	sys_info();

	return 0;
}
