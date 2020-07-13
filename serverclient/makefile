CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -g
NODE = node
SERVER = routing_server

all: $(SERVER)

server:
	$(CC) $(CFLAGS) routing_server.c -o routing_server

node:
	$(CC) $(CFLAGS) node.c -o node

util:
	$(CC) $(CFLAGS) util.c -o util

run: clean all
	./routing_server 1 4

clean:
	rm -f $(SERVER)
