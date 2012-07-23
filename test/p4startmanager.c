#include "setup.h"

void main(){
  /*Exec Manager First*/
  Exec("../test/manager", sizeof("../test/manager"), "manager thread", sizeof("manager thread"));
  Exit(0);
}
  
