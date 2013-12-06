#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket.h"

void startServer(int i){
  if(!fork()){
    startSocket(i);
  }
}

int main(int argc, const char *argv[])
{
  if (argc != 3) {
    printf("usage: server <port> <number of ports to open> \n");
    exit(0);
  }

  int i = atoi(argv[1]);
  int count = atoi(argv[2]) + i;

  while(i < count) {
    printf("Starting chat server on port %d\n", i);
    startServer(i++);
  }

  pause();

  return 0;
}
