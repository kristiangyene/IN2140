CC = gcc
CFLAGS = -g -ggdb -Wall -Wextra 

BINARIES = client server

CLIENT = client
SERVER = server


all: $(BINARIES)


server: server.c pgmread.c send_packet.c
	$(CC) $(CFLAGS) $^ -o $@


client: client.c send_packet.c linked_list.c
	$(CC) $(CFLAGS) $^ -o $@


run_client:
	./$(CLIENT) 127.0.0.1 2020 list_of_filenames.txt 0.1


run_server:	
	./$(SERVER) 2020 big_set matched_files.txt


clean:
	rm -f $(BINARIES)


val_client:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --trace-children=yes ./client 127.0.0.1 2020 list_of_filenames.txt 0.1


val_server:
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --trace-children=yes ./server 2020 big_set matched_files.txt



