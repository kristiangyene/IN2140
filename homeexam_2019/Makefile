CCFLAGS= -g

all: routing_server node pgmread

routing_server: routing_server.c util.c protocol.c print_lib.c
	gcc $(CFLAGS) $^ -o $@

node: node.c util.c protocol.c print_lib.c
	$(CC) $(CFLAGS) $^ -o $@


run: all
	bash run.sh

clean:
	rm -f routing_server
	rm -f node
