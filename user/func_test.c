#include "func_test.h"

int test_run(int n);

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
