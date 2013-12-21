#include "socket.h"

typedef struct client {
  int connfd;
  char connected;
  int index;
  pthread_t* thread;
}client;

const int MAX_CLIENTS = 200;
client* clients[MAX_CLIENTS];
int clients_index = 0;

/**
 * Send message to all clients except the one with the connfd specified in the third argument
 */
void send_to_all(char* message, int size, int connfd){
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if(clients[i] && clients[i]->connfd != connfd){
      printf("Sending to %d\n", i);

      if(!~send(clients[i]->connfd, message, size, 0)){
        fprintf(stderr, "%s%d\n", "Warning: Error sending message to client ", i);

        // unpolitely disconnect the client
        clients[i]->connected = 0;
      }
    }
  }
}

void talk_to_client(const void* d){

  char sendBuff[1025];
  memset(sendBuff, '0', sizeof(sendBuff));

  client* c = (client *) d;

  // don't crash on SIGPIPE
  signal(SIGPIPE, SIG_IGN);

  int bufsize=1024;
  char *receive_buffer=malloc(bufsize);
  while (c->connected) {
    int read_bytes = recv(c->connfd,receive_buffer,bufsize,0);
    if(read_bytes <= 0){
      printf("Slot %d seems to have disconnected.\n", c->index);
      break;
    }

    // HANDSHAKE
    if(strstr(receive_buffer, "Upgrade: websocket")){
      printf("Received Handshake request\n");

      // generate handshake response
      char* response = ws_handshake(receive_buffer);

      if(!~send(c->connfd, response, strlen(response), 0)){
        fprintf(stderr, "%s\n", "Warning: Error sending handshake to client!");
      }

      free(response);
      memset(receive_buffer, '0', strlen(receive_buffer));
    }else{

      // "parse" the incoming data into a frame
      // TODO make a function in ws
      struct bit_frame* in = (struct bit_frame*) &receive_buffer[0];

      // skip if message is too long
      // TODO implement handling of longer messages
      if(in->PAYLOAD > 125){
        continue;
      }

      char* decoded = ws_get_message(receive_buffer);

      // if it is actually a message and not some junk
      // TODO make a less implicit way of finding this out
      if(!strstr(decoded, "|")){
        continue;
      }

      // response frame size
      int frame_size = 2;

      // copy the frame for the response
      // TODO: might be unneccessary
      struct bit_frame frame;
      memcpy(&frame, in, frame_size);

      // server responses are not masked
      frame.MASK = 0;

      // our new message
      char message [frame_size + in->PAYLOAD + 1];

      // copy the response frame
      memcpy(message, &frame, frame_size);

      // copy the response message
      memcpy(&message[frame_size], &decoded[0], frame.PAYLOAD + 1);

      printf("Slot %d sending a message...\n", c->index);
      // send message to each client that is not our client
      // ATTENTION in->PAYLOAD is without + 1 on purpose, to avoid decoding errors
      send_to_all(message, frame_size + in->PAYLOAD, c->connfd);

      free(decoded);
      memset(receive_buffer, '0', strlen(receive_buffer));
    }
  }

  printf("Terminating thread %d\n", c->index);
  shutdown(c->connfd, 2);
  clients[c->index] = 0;
  free(c->thread);
  free(c);
  free(receive_buffer);
  pthread_exit(0);
}

void startSocket(const int port){

  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(!~listenfd){
    fprintf(stderr, "%s\n", "Fatal: Cannot open socket!");
    exit(1);
  }

  // tell socket we want to reuse it (enables easy restart)
  int opt = 1;
  setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR, (char *)&opt,sizeof(opt));

  // fill with zeroes
  memset(&serv_addr, '0', sizeof(serv_addr));

  // boilerplate bla
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  if(!~bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
    fprintf(stderr, "%s\n", "Fatal: Cannot bind to socket!");
    exit(1);
  }

  if(!~listen(listenfd, 10)){
    fprintf(stderr, "%s\n", "Fatal: Cannot listen on socket!");
    exit(1);
  }

  // don't crash on SIGPIPE
  signal(SIGPIPE, SIG_IGN);

  while(1){
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    if(!~connfd){
      fprintf(stderr, "%s\n", "Warning: A socket connection failed!");
    }else{
      printf("A Client connected\n");
    }


    // create new client
    client* c = malloc(sizeof(client));
    c->connfd = connfd;
    c->connected = 1;

    // check for empty slot in client array
    while(clients[clients_index]){
      clients_index++;
      if(clients_index == MAX_CLIENTS){
        clients_index = 0;
      }
    }

    printf("Put client in slot %d\n", clients_index);

    // put client in array
    c->index = clients_index;
    clients[clients_index] = c;

    // start new thread
    pthread_t* thread1 = malloc(sizeof(pthread_t));
    c->thread = thread1;
    if (pthread_create (thread1, NULL, (void *) talk_to_client, (void *) c)) {
      fprintf(stderr, "%s\n", "Fatal: Cannot create thread!");
      exit(1);
    }

    sleep(1);
  }
}

