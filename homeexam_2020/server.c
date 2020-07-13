#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>

#include "pgmread.h"
#include "send_packet.h"
#include "header.h"

#define BUFFER 1518 /*Maximum ethernet frame*/

struct payload{
    int request_number;
    int filename_length;
    char *filename;
    char *bytes;
}__attribute__((packed));

int port;
char *image_dir;
char *output_file;
unsigned char seq = 0;


void send_ACK_packet(int so, struct sockaddr_in client_addr){
    size_t packet_size = PACKET_HEADER_SIZE;

    /*Header structure*/
    struct packet_header* header = malloc(PACKET_HEADER_SIZE);
    header->len = PACKET_HEADER_SIZE;
    header->seq = seq;
    header->last_seq = seq;
    header->flags = 2;
    header->unused = 0x7f;

    /*Packet of bytestream*/
    char* packet = malloc(packet_size);
    memcpy(&(packet[0]), &(header->len), sizeof(int));
    memcpy(&(packet[4]), &(header->seq), sizeof(unsigned char));
    memcpy(&(packet[5]), &(header->last_seq), sizeof(unsigned char));
    memcpy(&(packet[6]), &(header->flags), sizeof(unsigned char));
    memcpy(&(packet[7]), &(header->unused), sizeof(unsigned char));

    free(header);
    
    int packet_len;
    memcpy(&(packet_len), &(packet[0]), sizeof(int));

    printf("Sending ACK packet for packet sequence %d...\n\n", seq);
    sendto(so, packet, packet_len, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
    free(packet);
}


/*-------------------------------------------------------------------------*/

void write_output(char* in_file, char* local_file){
    FILE *output = fopen(output_file, "a");
    /*Already checked if file exists*/
    fprintf(output, "\"%s %s\"\n", in_file, local_file);
    fclose(output);
}


/*-------------------------------------------------------------------------*/

int compare_images(char* payload, int packet_len){
    int unique;
    int len;
    char* filename;
    char* bytes;

    /*Packing up the payload recieved*/
    memcpy(&(unique), &(payload[0]), sizeof(int));
    memcpy(&(len), &(payload[4]), sizeof(int));

    filename = malloc(sizeof(char) * len);
    memcpy(filename, &(payload[8]), sizeof(char) * len);

    bytes = malloc(sizeof(char) * (packet_len - PACKET_HEADER_SIZE - (sizeof(int) * 2) - (sizeof(char) * len)));
    memcpy(bytes, &(payload[8 + len]), sizeof(char) * (packet_len - PACKET_HEADER_SIZE - (sizeof(int) * 2) - (sizeof(char) * len)));

    /*Make recieved image struct*/
    struct Image* rec_image = Image_create(bytes);
    free(bytes);
    
    struct dirent *pDirent;
    DIR *pDir;

    /*Ensure we can open directory*/
    pDir = opendir(image_dir);
    if (pDir == NULL){
        perror("opendir");
        return EXIT_FAILURE;
    }

    int similar = 0;
    int found = 0;

    /*Process each entry*/
    while ((pDirent = readdir(pDir)) != NULL){
        if (!strcmp (pDirent->d_name, "."))
            continue;
        if (!strcmp (pDirent->d_name, ".."))    
            continue;

        /*Get filename with path to fopen*/
        char* filename_path = malloc(strlen(image_dir) + strlen(pDirent->d_name) + 1 + strlen("/") + 1);
        strcpy(filename_path, image_dir);
        strcat(filename_path, "/");  
        strcat(filename_path, pDirent->d_name);    

        FILE *dir_file = fopen(filename_path, "r");
        if(dir_file == NULL){
            perror("fopen");
            return EXIT_FAILURE;
        }
        free(filename_path);
        int fd = fileno(dir_file);

        /*Collecting the file size*/
        struct stat file_info;
        fstat(fd, &file_info);

        /*Make image struct from directory*/
        char* dir_bytes = malloc((sizeof(char) * file_info.st_size));
        fread(dir_bytes, file_info.st_size, 1, dir_file);
        fclose(dir_file);

        struct Image* dir_image = Image_create(dir_bytes);
        free(dir_bytes);

        /*Compare the two image structs*/
        similar = Image_compare(rec_image, dir_image);
        Image_free(dir_image);
        
        if(similar == 1){
            found = 1;
            write_output(filename, pDirent->d_name);
        }
    }
    Image_free(rec_image);
    if(found == 0){
        write_output(filename, "UNKNOWN");
    }
    free(filename);
    
    /*Close directory and exit*/
    closedir(pDir);
    return 0;
}


/*-------------------------------------------------------------------------*/

void process_packet(char *packet){
    /*Unpack the header*/
    int packet_len;
    unsigned char seq;
    unsigned char last_seq;
    unsigned char flag;
    unsigned char unused;

    memcpy(&(packet_len), &(packet[0]), sizeof(int));
    memcpy(&(seq), &(packet[4]), sizeof(unsigned char));
    memcpy(&(last_seq), &(packet[5]), sizeof(unsigned char));
    memcpy(&(flag), &(packet[6]), sizeof(unsigned char));
    memcpy(&(unused), &(packet[7]), sizeof(unsigned char));

    /*Payload*/
    char *payload = malloc(packet_len - PACKET_HEADER_SIZE);
    memcpy(payload, &(packet[8]), packet_len - PACKET_HEADER_SIZE);

    /*Send payload for image comparing*/
    compare_images(payload, packet_len);

    free(payload);
}


/*-------------------------------------------------------------------------*/

/*Command arguments:
 *Port number, image-file directory, output file*/
int main(int argc, char *argv[]){
    if(argc != 4){
        perror("argc");
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    image_dir = argv[2];
    output_file = argv[3];

    int so, rc, yes = 0;

    struct in_addr ipadresse;
    struct sockaddr_in serveraddr, clientaddr;

    /*Listens to localhost*/
    inet_pton(AF_INET, "127.0.0.1", &ipadresse);

    /*Creating socket*/
    so = socket(AF_INET, SOCK_DGRAM, 0);
    if (so == -1){
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&serveraddr, 0, sizeof(serveraddr)); 
    memset(&clientaddr, 0, sizeof(clientaddr)); 

    /*Fill server info*/
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr = ipadresse;

    setsockopt(so, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    /*Socket listens to localhost and port sent as argument*/
    rc = bind(so, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in));
    if (rc == -1){
        perror("bind");
        return EXIT_FAILURE;
    }

    /*Clear output file of data*/
    FILE *output = fopen(output_file, "w");
    if(output == NULL){
        perror("fopen");
        return EXIT_FAILURE;
    }
    fclose(output);


    /*Recieving image data*/
    int n;
    socklen_t sock_len = sizeof(clientaddr);
    
    unsigned char recieved_seq;
    unsigned char flag = 0;

    /*While forever*/
    while(1){
        /*Recieve packets*/
        char *packet = malloc(PACKET_HEADER_SIZE + sizeof(char) * BUFFER);
        n = recvfrom(so, (char *)packet, BUFFER,  
                    0, ( struct sockaddr *) &clientaddr, &sock_len); 
        packet[n] = '\0'; 
        memcpy(&(recieved_seq), &(packet[4]), sizeof(unsigned char));
        memcpy(&(flag), &(packet[6]), sizeof(unsigned char));
        /*Check for termination*/
        if(flag & 0x4){
            printf("Termination packet recieved!\n");
            free(packet);
            return EXIT_SUCCESS;
        }
        /*If the correct sequence number is recieved*/
        if(recieved_seq == seq){
            printf("Received expected packet with sequence number %d!\n", recieved_seq);
            process_packet(packet);
            send_ACK_packet(so, clientaddr);
            seq++;
        }
        free(packet);
    }
    close(so);
    
    return EXIT_SUCCESS;
}
