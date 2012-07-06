#include "../userprog/syscall.h"

int main(){
	NPrint("Testing exec 2 matmults...\n", sizeof("Testing exec 2 matmults...\n"));
	Exec("../test/matmult", sizeof("../test/matmult"), "matmult 1", sizeof("matmult 1"));
	Exec("../test/matmult", sizeof("../test/matmult"), "matmult 2", sizeof("matmult 2"));
	Exit(0);
}
