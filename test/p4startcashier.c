#include "setup.h"

void main(){
    /*Cashiers Next*/
    Exec("../test/cashier", sizeof("../test/cashier"), "cashier0 thread", sizeof("cashier0 thread"));
    Exec("../test/cashier", sizeof("../test/cashier"), "cashier1 thread", sizeof("cashier1 thread"));
    Exec("../test/cashier", sizeof("../test/cashier"), "cashier2 thread", sizeof("cashier2 thread"));

    Exit(0);
  
}
