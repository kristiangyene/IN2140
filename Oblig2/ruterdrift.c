#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>


struct router{
    unsigned char id;
    char* flag;
    char* model;
};

int N;
struct router **routers;



void tobinary(char* flaggchar, int flagg, int count){
  if(count >= 0){
    double buffer = (double)flagg/2;
    int buff = (int)buffer;

    if(buffer-buff != 0){
      flaggchar[count] = '1';
    }
    tobinary(flaggchar, buff, count-1);
  }
}


void read_data(FILE *f){
    fgetc(f);
    unsigned char id =(unsigned char) fgetc(f);


    int flagg = fgetc(f);

    char* flaggchar = (char*)malloc(sizeof(char)*9);
    strcpy(flaggchar, "00000000");
    tobinary(flaggchar, flagg, 7);


    int stringlengde = fgetc(f);


    char* ruternavn = (char*)malloc(sizeof(char)*(stringlengde));
    fgets(ruternavn, stringlengde, f);

    struct router* router = malloc(sizeof(struct router*));
    
    router->id = id;
    router->flag = flaggchar ;
    router->model = ruternavn;

    routers[id] = router;




}


int main(int argc, char* argv[]){
    if(argc != 2){
        perror("argc");
        printf("Useage: \n\t%s router_file [kommando_file]\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *f = fopen(argv[1], "rb");

    if(f == NULL){
        perror("fopen");
        return EXIT_FAILURE;
    }

    //Extracting number of routers
    fread(&N, 4, 1, f);
    printf("Number of routers in this network: %d.\n", N);

    for(int i = 1; i < N; i++){
        read_data(f);
    }

    return EXIT_SUCCESS;

}
