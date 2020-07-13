#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE strlen(RESPONSE_SUCCESS)
#define SERVER_PORT 21400

int main(){
	char buffer[BUFFER_SIZE];
	int client_socket;
	struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    LOG(LOGGER_DEBUG, "Setting up connection socket");

	client_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_socket == -1) {
        LOG(LOGGER_ERROR, "socket() failed");
		perror("socket");
		return 1;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        /* TODO */
    sendto(client_socket, RESPONSE_SUCCESS, RESPONSE_SUCCESS_LEN, NULL, &server_addr, server_addr_len);

	close(client_socket);
}
