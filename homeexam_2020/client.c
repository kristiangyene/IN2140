#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <time.h>
#include <libgen.h>

#include "send_packet.h"
#include "header.h"
#include "linked_list.h"

#define MAX_WINDOW_SIZE 7
#define TIMEOUT_SECS    5
#define BUFFER          1518 /*Maximum ethernet frame*/


int server_port;
char *server_ip;
int request_number = 100; 
unsigned char  packets_acked = 0;
unsigned char seq = 0;
unsigned char last_rec_seq = 0;
int number_of_files;



void send_termination_packet(int so){
    size_t packet_size = PACKET_HEADER_SIZE;

    /*Header structure*/
    struct packet_header* header = malloc(PACKET_HEADER_SIZE);
    header->len = PACKET_HEADER_SIZE;
    header->seq = seq++;
    header->last_seq = last_rec_seq;
    header->flags = 4;
    header->unused = 0x7f;

    /*Packet of bytestream*/
    char* packet = malloc(packet_size);
    memcpy(&(packet[0]), &(header->len), sizeof(int));
    memcpy(&(packet[4]), &(header->seq), sizeof(unsigned char));
    memcpy(&(packet[5]), &(header->last_seq), sizeof(unsigned char));
    memcpy(&(packet[6]), &(header->flags), sizeof(unsigned char));
    memcpy(&(packet[7]), &(header->unused), sizeof(unsigned char));

    free(header);

    /*Setting server info*/
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    int packet_len;
    memcpy(&(packet_len), &(packet[0]), sizeof(int));

    printf("Sending termination packet...\n\n");
    sendto(so, packet, packet_len, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    free(packet);
}


/*-------------------------------------------------------------------------*/

char* create_image_packet(FILE *f, char* filename){
    int fd = fileno(f);

    /*Collecting the file size*/
    struct stat file_info;
    fstat(fd, &file_info);

    int filename_length = strlen(filename) + 1;
    
    /*Collecting image bytes*/
    char* bytes = malloc((sizeof(char) * file_info.st_size));
    fread(bytes, file_info.st_size, 1, f);

    size_t payload_size = ((sizeof(int) * 2) + (sizeof(char) * filename_length) + (sizeof(char) * file_info.st_size));

    /*Payload bytestream*/
    char* payload = malloc(payload_size);
    memcpy(&(payload[0]), &request_number, sizeof(int));
    memcpy(&(payload[4]), &filename_length, sizeof(int));
    memcpy(&(payload[8]), filename, sizeof(char) * filename_length);
    memcpy(&(payload[8 + filename_length]), bytes, sizeof(char) * file_info.st_size);
    request_number++;
    free(bytes);

    /*Header structure*/
    struct packet_header* header = malloc(PACKET_HEADER_SIZE);
    header->len = PACKET_HEADER_SIZE + payload_size;
    header->seq = seq;
    header->last_seq = last_rec_seq;
    header->flags = 1;
    header->unused = 0x7f;
    seq = seq + 1;
    

    size_t packet_size = PACKET_HEADER_SIZE + payload_size;

    /*Packet of bytestream*/
    char* packet = malloc(packet_size);
    memcpy(&(packet[0]), &(header->len), sizeof(int));
    memcpy(&(packet[4]), &(header->seq), sizeof(unsigned char));
    memcpy(&(packet[5]), &(header->last_seq), sizeof(unsigned char));
    memcpy(&(packet[6]), &(header->flags), sizeof(unsigned char));
    memcpy(&(packet[7]), &(header->unused), sizeof(unsigned char));
    memcpy(&(packet[8]), payload, payload_size);

    free(payload);
    free(header);

    printf("Packet with filename '%s' successfully made!\n", filename);
    return packet;
}


/*-------------------------------------------------------------------------*/

void send_image_file(FILE* f, char* filename, int so){
    /*make package*/
    char *packet = create_image_packet(f, filename);

    /*Setting server info*/
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    int packet_len;
    memcpy(&(packet_len), &(packet[0]), sizeof(int));

    printf("Sending packet with %d bytes...\n\n", packet_len);
    send_packet(so, packet, packet_len, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    free(packet);
}

/*-------------------------------------------------------------------------*/


void resend_packages(linked_list *not_acked, int so){
    int i = 0;
    while(i < linked_list_size(not_acked)){
        packet* packet = get(not_acked, i);
        /*Resend packets*/
        FILE* f = fopen(packet->data, "r");
        /*Check if filename is unvalid*/
        if(f == NULL){
            perror("fopen resend");
            return;
        }
        const char* base = basename(packet->data);
        char* filename = malloc(strlen(base) + 1);
        strcpy(filename, base);

        send_image_file(f, filename, so);
        free(filename);
        fclose(f);
        i++;
    }
}


/*-------------------------------------------------------------------------*/
/* Main command arguments:
 * server addr, port number, image filenames, drop percentage*/
int main(int argc, char *argv[]){


    if(argc < 5){
        perror("argc");
        return EXIT_FAILURE;
    }

    server_ip = argv[1];
    server_port = atoi(argv[2]);

    char *filename_list = argv[3];

    /*Get number of filenames*/
    FILE *line_count = fopen(filename_list, "r");
    /*Check if filename is unvalid*/
    if(line_count == NULL){
        perror("fopen");
        return EXIT_FAILURE;
    }
    /*read character by character and check for new line*/
    char ch;
    while((ch=fgetc(line_count))!=EOF){
        if(ch=='\n')
            number_of_files++;
    }
    //close the file.
    fclose(line_count);

    /*File containing filenames*/
    FILE *f_list = fopen(filename_list, "r");

    /*Set packet loss percentage*/
    float drop_percentage = atof(argv[4]);
    set_loss_probability(drop_percentage);


    /*Setting up sockets*/
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr)); 
    memset(&client_addr, 0, sizeof(client_addr)); 
    
     /*Creating socket that sends data with IPv4*/
    int so = socket(AF_INET, SOCK_DGRAM, 0);

    if (so == 0) {
        perror("socket");
        fclose(f_list);
        return EXIT_FAILURE;
    }
    /*Fill server info*/
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    /*Setup for go-back-n*/
    char* line = NULL;
    size_t len = 0;
    int read;
    linked_list *not_acked = new_linked_list();
    socklen_t sock_len = sizeof(server_addr);
    unsigned char recieved_seq = 0;

    /*Send 7 packet and fill window to max window size*/
        while(linked_list_size(not_acked) < MAX_WINDOW_SIZE){
            read = getline(&line, &len, f_list);
            if(read != -1){
                strtok(line, "\n"); //Remove newline
                FILE *f = fopen(line, "r");
                /*Check if filename is unvalid*/
                if(f == NULL){
                    perror("fopen");
                    return EXIT_FAILURE;
                }
                /*Remove path*/
                const char* base = basename(line);
                char* filename = malloc(strlen(base) + 1);
                strcpy(filename, base);

                /*Send packet and put in linked list*/
                send_image_file(f, filename, so);
                append(not_acked, line);
                fclose(f);
                free(filename);
            }
        }


    /*While more packets can be sent*/
    while(recieved_seq != number_of_files-1){
        struct timeval tv = {5, 0}; /*Setting timeout for 5s*/
        fd_set set;
        FD_ZERO(&set);
        FD_SET(so, &set);

        int ret = select(FD_SETSIZE, &set, NULL, NULL, &tv);
        if(ret == -1){
            perror("select");
            return EXIT_FAILURE;
        }
        else if(ret){ 
            /*Recieving ACK packet*/
            char* packet = malloc(PACKET_HEADER_SIZE);

            recvfrom(so, (char *)packet, PACKET_HEADER_SIZE,  
            0, ( struct sockaddr *) &server_addr, &sock_len);
            memcpy(&(recieved_seq), &(packet[4]), sizeof(unsigned char));

            /*ACK recieved*/
            printf("Received ACK packet with sequence number %d!\n\n", recieved_seq);

            /*If the expected ACK packet is recieved*/
            if(recieved_seq == packets_acked){
                /*Remove the last sent but not acked*/
                pop(not_acked);
                free(packet);
                read = getline(&line, &len, f_list);

                if(read != -1){
                    strtok(line, "\n");
                    FILE *f = fopen(line, "r");
                    /*Check if filename is unvalid*/
                    if(f == NULL){
                        perror("fopen");
                        return EXIT_FAILURE;
                    }
                    /*Remove path*/
                    const char* base = basename(line);
                    char* filename = malloc(strlen(base) + 1);
                    strcpy(filename, base);

                    /*Send and advance with one packet*/
                    send_image_file(f, filename, so);
                    append(not_acked, line);
                    fclose(f);
                    free(filename);
                }
            }
            packets_acked++;
        }
        else{
            /*Need to send not acked packets again*/
            seq = packets_acked; /*Set seq to seq for lost packet*/
            resend_packages(not_acked, so);
        }
    }
    /*All packets are sent*/
    send_termination_packet(so);
    free_linked_list(not_acked);
    free(line);
    fclose(f_list);
    close(so);

    return EXIT_SUCCESS;
}
