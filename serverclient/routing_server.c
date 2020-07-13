#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

#include "util.c"

#define IN 99

//routing_server <P> <N>
//<P> TCP porten som serveren lytter til
//<N> Antall noder


struct edge{
  int weight;
};

struct node{
  uint16_t address;
  struct edge** edges;
  struct node** neighbours;
  char *ip;
  unsigned short port;
};

int number_of_nodes;


/*int dijkstra_algorithm(int cost[][number_of_nodes],int source,int target){
    int dist[number_of_nodes],prev[number_of_nodes],selected[number_of_nodes]={0},i,m,min,start,d,j;
    char path[number_of_nodes];
    for(i=1;i< number_of_nodes;i++){
        dist[i] = IN;
        prev[i] = -1;
    }
    start = source;
    selected[start]=1;
    dist[start] = 0;
    while(selected[target] ==0){
        min = IN;
        m = 0;
        for(i=1;i< number_of_nodes;i++){
            d = dist[start] +cost[start][i];
            if(d< dist[i]&&selected[i]==0){
                dist[i] = d;
                prev[i] = start;
            }
            if(min>dist[i] && selected[i]==0){
                min = dist[i];
                m = i;
            }
        }
        start = m;
        selected[start] = 1;
    }
    start = target;
    j = 0;
    while(start != -1){
        path[j++] = start+65;
        start = prev[start];
    }
    path[j]='\0';
    strrev(path);
    printf("%s", path);
    return dist[target];
  }*/


int create_tcp_socket(){
  int binding, sockopt, socket = 1;
  struct sockaddr_in server_addr;

   LOG("DEBUG", "Creating routing_server socket.");
  //Returnerer en descriptor.
  socket = socket(AF_INET, SOCK_STREAM, 0);
  if(socket == -1){
    LOG("ERROR", "Failed to create routing_server socket.");
        perror("socket");
        exit(EXIT_FAILURE);
  }
  LOG("DEBUG", "Created server socket");
}


int main(int argc, char* argv[]){
  if(argc != 3){
      printf("%s requires 2 arguments.\n", argv[0]);
      exit(EXIT_FAILURE);
  }
  number_of_nodes = atoi(argv[2]);
  create_tcp_socket();
  return EXIT_SUCCESS;

}
