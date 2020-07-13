#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "linked_list.h"


/*Initializing a dynamically allocated linked_list*/
linked_list* new_linked_list(){
    linked_list* temp = malloc (sizeof* temp);  
    if (!temp){                     
        perror("malloc");
        return NULL;
    }
    temp->head = NULL; 
    temp->size = 0;

    return temp;
}

/*-------------------------------------------------------------------------*/


/*Create new packet*/
packet* create_new_packet(char* data){
    packet* new_packet = NULL;

    if (!data){ 
        perror("data");
        return NULL;
    }

    new_packet = malloc(sizeof* new_packet); 
    if(!new_packet){
        perror("malloc");
        return NULL;
    }

    if(!(new_packet->data = malloc(strlen (data) + 1))){
        perror("malloc");
        free(new_packet);
        return NULL;
    }
    strcpy(new_packet->data, data); 
    new_packet->next = NULL;

    return new_packet;   
}

/*-------------------------------------------------------------------------*/


/*Insert a packet at the end of the list*/
void append(linked_list* linked_list, char* data){ 
    packet* new_packet = create_new_packet(data);
    packet* current = linked_list->head;

    if(!new_packet) return;

    /*append in empty list*/
    if(current == NULL){
        new_packet->next = current;
        linked_list->head = new_packet;
    }
    else{
        /*find last element*/
        packet* temp = current;
        while (1){ 
            if(temp->next == NULL){
                temp->next = new_packet;
                break; 
            }
            temp = temp->next;
        };
    }
    linked_list->size++;
}

/*-------------------------------------------------------------------------*/


/*Remove the head of the linkes list*/
void pop(linked_list* linked_list){
    packet* current = linked_list->head;
    if (current != NULL){
        linked_list->head = current->next;
        current->next = NULL;
        free(current->data);
        free(current);
    }
}

/*-------------------------------------------------------------------------*/


/*Get packet at a specific index*/
packet* get(linked_list* linked_list, int index){ 
    packet* current = linked_list->head;
      
    int count = 0; 
    while(current != NULL){ 
        if(count == index){
            return current; 
        }
        count++; 
        current = current->next; 
    } 
    /*If index is none-existing*/
    assert(0);              
} 

/*-------------------------------------------------------------------------*/


/*Tostring*/
void print_linked_list(linked_list* linked_list){
    packet* current = linked_list->head;

    while (current != NULL){
        if (current == linked_list->head)
            printf("%s", current->data);
        else
            printf(", %s", current->data);
        current = current->next;
    }
    putchar('\n');
}

/*-------------------------------------------------------------------------*/


/*Get the current size of the linked list*/
int linked_list_size(linked_list* linked_list){
    packet* current = linked_list->head;
    int i = 0;
    while(current != NULL){
        current = current->next;
        i++;
    }
    return i;
}

/*-------------------------------------------------------------------------*/


/*Free all packets and the linked list itself*/
void free_linked_list(linked_list* linked_list){
    packet* current = linked_list->head;
    while(current != NULL){
        packet* temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
    free(linked_list);
}

