#include "setup.h"

void main(){
  /*Salesmen Next*/
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman0-0 thread", sizeof("salesman0-0 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman0-1 thread", sizeof("salesman0-1 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman0-2 thread", sizeof("salesman0-2 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman1-0 thread", sizeof("salesman1-0 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman1-1 thread", sizeof("salesman1-1 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman1-2 thread", sizeof("salesman1-2 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman2-0 thread", sizeof("salesman2-0 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman2-1 thread", sizeof("salesman2-1 thread"));
  Exec("../test/salesman", sizeof("../test/salesman"), "salesman2-2 thread", sizeof("salesman2-2 thread"));
  Exit(0);
}
