#include "socket.h"

typedef struct client {
  int connfd;
  char connected;
  int index;
  pthread_t* thread;
}client;

time_t ticks;
client* clients[100];
int clients_index = 0;

void talk_to_client(void* d){

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
      printf("Error in %d?\n", c->index);
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

      // cleanup
      free(response);
      memset(receive_buffer, '0', strlen(receive_buffer));
    }else{

      // "parse" the incoming data into a frame
      // TODO make a function in ws
      struct bit_frame* in = (struct bit_frame*) &receive_buffer[0];

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

      memcpy(message, &frame, frame_size);
      memcpy(&message[frame_size], &decoded[0], frame.PAYLOAD + 1);

      printf("We are in thread %d\n", c->index);
      // send message to each client that is not our client
      for (int i = 0; i < clients_index; i++) {
        if(clients[i] && clients[i]->connfd != c->connfd){
          printf("Sending to %d\n", i);

          // ATTENTION in->PAYLOAD is without + 1 on purpose, to avoid decoding errors
          if(!~send(clients[i]->connfd, message, frame_size + in->PAYLOAD, 0)){
            fprintf(stderr, "%s%d\n", "Warning: Error sending message to client ", i);
            clients[i]->connected = 0;
          }
        }
      }

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

void startSocket(int port){

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


    client* c = malloc(sizeof(client));
    c->connfd = connfd;
    c->connected = 1;
    c->index = clients_index;
    clients[clients_index++] = c;

    pthread_t* thread1 = malloc(sizeof(pthread_t));
    c->thread = thread1;
    if (pthread_create (thread1, NULL, (void *) talk_to_client, (void *) c)) {
      fprintf(stderr, "%s\n", "Fatal: Cannot create thread!");
      exit(1);
    }

    sleep(1);
  }
}

