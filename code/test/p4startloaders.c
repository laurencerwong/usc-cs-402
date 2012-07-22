#include "setup.h"

void main(){
    
  /*Goodsloaders Next*/
  Exec("../test/goodsloader", sizeof("../test/goodsloader"), "goodsloader0 thread", sizeof("goodsloader0 thread"));
  Exec("../test/goodsloader", sizeof("../test/goodsloader"), "goodsloader1 thread", sizeof("goodsloader1 thread"));
  Exec("../test/goodsloader", sizeof("../test/goodsloader"), "goodsloader2 thread", sizeof("goodsloader2 thread"));

  Exit(0);
}
