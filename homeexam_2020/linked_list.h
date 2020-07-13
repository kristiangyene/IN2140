#ifndef LINKED_LIST_H
#define LINKED_LIST_H


typedef struct packet{
    char* data;
    struct packet *next;
} packet;

typedef struct linked_list{  
    packet* head;
    size_t size;       
} linked_list;


/*Linked list methods*/
linked_list* new_linked_list();
packet* create_new_packet( char* data);
void append(linked_list* linked_list, char* data);
void pop(linked_list* linked_list);
packet* get(linked_list* linked_list, int index);
void print_linked_list(linked_list* linked_list);
int linked_list_size(linked_list* linked_list);
void free_linked_list(linked_list* linked_list);


#endif /* LINKED_LIST_H */
