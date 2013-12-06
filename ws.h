#ifndef _WS_H
#define _WS_H
#include <string.h>
#include <stdlib.h>
#include "crypto.h"

typedef union ws_frame{
  struct bit_frame{
    unsigned int OP_CODE : 4;
    unsigned int RSV1 : 1;
    unsigned int RSV2 : 1;
    unsigned int RSV3 : 1;
    unsigned int FIN : 1;
    unsigned int PAYLOAD : 7;
    unsigned int MASK : 1;
  };
  uint16_t raw_frame;
}ws_frame;

char* ws_handshake(const char* receive_buffer);

char* ws_get_message(const char* receive_buffer);

#endif
