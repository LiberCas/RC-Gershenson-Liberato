#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int mopen(){
    //Writing string
    /*printf("7/8: \n");
     
    unsigned char buf[3] = {0};
    fgets(buf, 3, stdin);
    if(buf[0] == '7'){*/
    int bytes = llopen(7, TRANSMITTER);
        
    /*
    if(buf[0] == '8'){
        int bytes = llopen(8, RECEIVER);
    }
    else{
        printf("%c", buf[0]);
    }
    printf("2 bytes written\n");*/
}