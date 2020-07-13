#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util.h"
#include "protocol.h"

#define MAX_NUM_NODES 10
#define BACKLOG_SIZE 10

//routing_server <Baseport P> <Number of nodes N>

struct node {
  unsigned char address;
  int socket;
  char *ip;
  unsigned short port;
};

struct node nodes[MAX_NUM_NODES] = { 0 };

int server_port;
int number_of_nodes;



int create_server_socket() {
  int ret;
  int yes = 1;
  int server_socket = 1;

  struct sockaddr_in server_addr;

  LOG(LOGGER_DEBUG, "Creating server socket");

  //Setter opp serversocket.
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    LOG(LOGGER_ERROR, "Failed to create socket");
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  LOG(LOGGER_DEBUG, "Created server socket");

  /*Setter opp hvilke tilkoblinger som skal aksepteres og hvilken port som
  skal brukes.*/
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port); //Lytter på baseporten.
  server_addr.sin_addr.s_addr = INADDR_ANY;

  //Gjør porten gjenbrukbar.
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  LOG(LOGGER_DEBUG, "Binding server socket to port %d", server_port);

  //Binder andressen til socket.
  ret = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (ret) {
    LOG(LOGGER_ERROR, "Failed to bind");
    perror("bind()");
    exit(EXIT_FAILURE);
  }

  LOG(LOGGER_DEBUG, "Server socket succesfully bound");
  LOG(LOGGER_DEBUG, "Trying to listen to socket");

  //Lytter på tilkoblinger.
  ret = listen(server_socket, BACKLOG_SIZE);
  if (ret == -1) {
    LOG(LOGGER_ERROR, "Failed to listen");
    perror("listed()");
    exit(EXIT_FAILURE);
  }

  LOG(LOGGER_DEBUG, "Listening on socket...");

  return server_socket;
}

void remove_node(int socket) {
  int i;

  LOG(LOGGER_DEBUG, "Node disconnection at socket: %d", socket);
  for (i = 0; i < number_of_nodes; ++i) {
    if (nodes[i].socket == socket) {
      LOG(LOGGER_INFO, "Disconnection from IP address %s and port %u\n", nodes[i].ip, nodes[i].port);
      nodes[i].socket = -1;
      free(nodes[i].ip);
      LOG(LOGGER_DEBUG, "Node found and removed");
      return;
    }
  }
  LOG(LOGGER_ERROR, "No node found at socket %d", socket);
}

int add_node(int socket, struct sockaddr_in *node_addr, socklen_t addrlen) {
    char *ip_buffer = malloc(16);
    int i;

    LOG(LOGGER_DEBUG, "Attempting to add connecting node to internal node list");

    if (!inet_ntop(node_addr->sin_family, &(node_addr->sin_addr), ip_buffer, addrlen)) {
      LOG(LOGGER_ERROR, "Error converting IP address to string");
      perror("inet_ntop");
      strcpy(ip_buffer, "N/A");
    }

    LOG(LOGGER_INFO, "Connection from IP address %s and port %u\n", ip_buffer, ntohs(node_addr->sin_port));

    for (i = 0; i < number_of_nodes; ++i) {
      //Tar imot ny node.
      if (nodes[i].socket == -1) {
        LOG(LOGGER_DEBUG, "Adding node and sending acceptance message");
        nodes[i].socket = socket;
        nodes[i].ip = ip_buffer;
        nodes[i].port = ntohs(node_addr->sin_port);

        LOG(LOGGER_DEBUG, "Writing response to node: %s", RESPONSE_SUCCESS);
        write(socket, RESPONSE_SUCCESS, strlen(RESPONSE_SUCCESS));
        return 0;
      }
    }
    free(ip_buffer);
    close(socket);
    return 1;
}

int handle_connection(int server_socket, fd_set *sockets){
  int node_socket;
  struct sockaddr_in node_addr;
  socklen_t addrlen;

  //Tar imot ny tilkobling.
  LOG(LOGGER_DEBUG, "New socket %d for other connections\n", server_socket);

  addrlen = sizeof(struct sockaddr_in);
  node_socket = accept(server_socket,
          (struct sockaddr*)&node_addr,
          &addrlen);

  if (node_socket == -1) {
    LOG(LOGGER_ERROR, "Failed to accept, continuing...");
    perror("accept()");
    return -1;
  }

  LOG(LOGGER_DEBUG, "Accepted node on Socket: %d", node_socket);
  if (add_node(node_socket, &node_addr, addrlen)) {
    return -1;
  }

  FD_SET(node_socket, sockets);
  return node_socket;
}




void run_server(int listener) {
  fd_set sockets;
  fd_set read_sockets;

  int largest_socket = listener;
  int i;

  FD_ZERO(&sockets);
  FD_SET(listener, &sockets);

  LOG(LOGGER_DEBUG, "Entering select loop");

  while (1) {
    read_sockets = sockets;

    LOG(LOGGER_DEBUG, "Calling select, blocking");
    if (select(largest_socket+1, &read_sockets, NULL, NULL, NULL) == -1) {
      LOG(LOGGER_ERROR, "Failed to select");
      perror("select()");
      return;
    }
    LOG(LOGGER_DEBUG, "Looping through sockets up to %d", largest_socket);
    for (i = 0; i <= largest_socket; ++i) {
      if (FD_ISSET(i, &read_sockets)) {

        LOG(LOGGER_DEBUG, "Activity at socket %d", i);
        if (i == listener) {
          int node_socket = handle_connection(i, &sockets);
          if(node_socket == -1){
            continue;

          } else if (node_socket > largest_socket) {
            LOG(LOGGER_DEBUG, "Updating largest socket...");
            largest_socket = node_socket;
          }
        } else {
            //handle_message(i, &fds);
        }
      }
    }
  }
}

int main(int argc, char* argv[]) {

  if (argc != 3) {
      LOG(LOGGER_ERROR, "Not enough arguments");
      exit(EXIT_FAILURE);
  }

  server_port = atoi(argv[1]);
  number_of_nodes = atoi(argv[2]);

  int i;
  int server_socket;

  //1 11:2 103:6
  //11 1:2 13:7 19:2
  //13 11:7 17:3 101:4
  //17 13:3 107:2
  //19 11:2 101:2 103:1
  //101 13:4 19:2 107:2
  //103 1:6 19:1 107:4
  //107 17:2 101:2 103:4

  server_socket = create_server_socket();

  for (i = 0; i < number_of_nodes; ++i){
      nodes[i].socket = -1;
  }
  run_server(server_socket);


  for (i = 0; i < number_of_nodes; ++i){
      if (nodes[i].socket != -1) {
          close(nodes[i].socket);
          free(nodes[i].ip);
      }
  }

  close(server_socket);
  return EXIT_SUCCESS;
}
