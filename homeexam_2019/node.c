#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util.h"
#include "protocol.h"

#define SERVER_PORT 6000

//node <Port> <OwnAddress> <NeighbourAddress>:<weight>...
void send_node_info(int node_socket, char* info_string){
  int nBytes = strlen(info_string) + 1;
  send(node_socket,info_string,nBytes,0);
  recv(node_socket, info_string, 1024, 0);
  printf("Received from server: %s\n\n", info_string);
}

int socket_setup(char* address) {
  int ret;
  struct sockaddr_in server_addr;
  int node_socket;

  node_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (node_socket == -1) {
      LOG(LOGGER_ERROR, "Failed to create socket");
      perror("socket()");
      return 0;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = inet_addr(address);

  LOG(LOGGER_DEBUG, "Connecting to server at %s, port %d", address, SERVER_PORT);

  ret = connect(node_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
  if (ret == -1) {
      LOG(LOGGER_ERROR, "Failed to connect to server");
      perror("connect()");
      return 0;
  }

  return node_socket;
}

int main(int argc, char* argv[]) {
  int node_socket;

  if (argc < 2) {
    LOG(LOGGER_ERROR, "Not enough arguments");
    exit(EXIT_FAILURE);
  }

  LOG(LOGGER_DEBUG, "Setting up connection socket");
  node_socket = socket_setup("192.168.1.6");
  if (node_socket == 0) {
    exit(EXIT_FAILURE);
  }
  for(int i = 3; i < argc; i++){
    send_node_info(node_socket, argv[i]);
  }
  printf("Press enter to continue\n");

  close(node_socket);
  return EXIT_SUCCESS;
}
