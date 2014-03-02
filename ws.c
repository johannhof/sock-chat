#include "ws.h"

const char* RESPONSE_TEXT = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
const char* KEY = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

char* ws_handshake(const char* receive_buffer){

  const char* index = strstr(receive_buffer, "Sec-WebSocket-Key:") + 19;
  const char* stop = strstr(index, "\r");
  char key[1024];
  strncpy(key, index, stop - index);
  strncat(key, KEY, 37);

  unsigned char hashed[200];
  SHA1(key, strlen(key), hashed);

  char* encoded = base64(hashed, strlen(hashed));

  char* response = malloc(1024);

  // build handshake response
  strncat(response, RESPONSE_TEXT, strlen(RESPONSE_TEXT));
  strncat(response, encoded, strlen(encoded));
  free(encoded);
  strcat(response, "\r\n\r\n");

  return response;
}

char* ws_get_message(const char* receive_buffer){

  // "parse" the incoming data into a frame
  struct bit_frame* in = (struct bit_frame*) &receive_buffer[0];
  unsigned char *keys = (unsigned char*) &receive_buffer[2];
  unsigned char *encoded = (unsigned char*) &receive_buffer[6];

  char* decoded = malloc(in->PAYLOAD + 1);

  // xor-decode the message
  for (int x = 0; x < in->PAYLOAD; x++) {
    decoded[x] = (char)(encoded[x] ^ keys[x % 4]);
  }

  // end the string
  decoded[in->PAYLOAD] = 0;
  return decoded;
}
