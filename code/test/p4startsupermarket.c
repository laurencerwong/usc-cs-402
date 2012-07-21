#include "setup.h"

void main(){
  /*Exec Manager First*/
  Exec("../test/manager", sizeof("../test/manager"), "manager thread", sizeof("manager thread"));

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


  /*Cashiers Next*/
  Exec("../test/cashier", sizeof("../test/cashier"), "cashier0 thread", sizeof("cashier0 thread"));
  Exec("../test/cashier", sizeof("../test/cashier"), "cashier1 thread", sizeof("cashier1 thread"));
  Exec("../test/cashier", sizeof("../test/cashier"), "cashier2 thread", sizeof("cashier2 thread"));

  /*Goodsloaders Next*/
  Exec("../test/goodsloader", sizeof("../test/goodsloader"), "goodsloader0 thread", sizeof("goodsloader0 thread"));
  Exec("../test/goodsloader", sizeof("../test/goodsloader"), "goodsloader1 thread", sizeof("goodsloader1 thread"));
  Exec("../test/goodsloader", sizeof("../test/goodsloader"), "goodsloader2 thread", sizeof("goodsloader2 thread"));

  /*Customers Next*/
  Exec("../test/customer", sizeof("../test/customer"), "customer0 thread", sizeof("customer0 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer1 thread", sizeof("customer1 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer2 thread", sizeof("customer2 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer3 thread", sizeof("customer3 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer4 thread", sizeof("customer4 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer5 thread", sizeof("customer5 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer6 thread", sizeof("customer6 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer7 thread", sizeof("customer7 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer8 thread", sizeof("customer8 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer9 thread", sizeof("customer9 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer10 thread", sizeof("customer10 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer11 thread", sizeof("customer11 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer12 thread", sizeof("customer12 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer13 thread", sizeof("customer13 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer14 thread", sizeof("customer14 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer15 thread", sizeof("customer15 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer16 thread", sizeof("customer16 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer17 thread", sizeof("customer17 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer18 thread", sizeof("customer18 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer19 thread", sizeof("customer19 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer20 thread", sizeof("customer20 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer21 thread", sizeof("customer21 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer22 thread", sizeof("customer22 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer23 thread", sizeof("customer23 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer24 thread", sizeof("customer24 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer25 thread", sizeof("customer25 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer26 thread", sizeof("customer26 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer27 thread", sizeof("customer27 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer28 thread", sizeof("customer28 thread"));
  Exec("../test/customer", sizeof("../test/customer"), "customer29 thread", sizeof("customer29 thread"));
		    
  Exit(0);
};
