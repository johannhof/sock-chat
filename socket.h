#ifndef SOCKET_SCRIPT
#define SOCKET_SCRIPT

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

#include "crypto.h"
#include "ws.h"

void startSocket(int port);

#endif
