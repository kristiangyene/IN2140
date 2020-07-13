#ifndef __PROTOCOL_H
#define __PROTOCOL_H


#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>



/*Header Size*/
#define PACKET_HEADER_SIZE (sizeof(uint32_t) + (sizeof(uint8_t) * 4))



/*Packet header*/
struct packet_header{
    uint32_t len;
    uint8_t seq;
    uint8_t last_seq;
    uint8_t flags;
    uint8_t unused;
}__attribute__((packed));



#endif
