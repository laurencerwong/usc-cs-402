#include "../userprog/syscall.h"

int myQueue;

void main(int argc, char** argv){
  myQueue = CreateQueue();
  NPrint("myQueueSize is: %d\n", sizeof("myQueueSize is: %d\n"), QueueSize(myQueue));
  QueuePush(myQueue, 10);
  NPrint("myQueueSize is: %d\n", sizeof("myQueueSize is: %d\n"), QueueSize(myQueue));
  
  
  Exit(0);
}
