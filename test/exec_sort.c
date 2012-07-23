#include "../userprog/syscall.h"

int main(){
	NPrint("Testing exec 2 sorts", sizeof("Testing exec 2 sorts"));
	Exec("../test/sort", sizeof("../test/sort"), "sort 1", sizeof("sort 1"));
	Exec("../test/sort", sizeof("../test/sort"), "sort 2", sizeof("sort 2"));
	Exit(0);
}
