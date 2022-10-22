#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int mopen(){
    //Writing string
    printf("3/5: \n");
     
    unsigned char buf[3] = {0};
    fgets(buf, 3, stdin);
   /*
    if(buf[0] == 3){
        int bytes = llopen(buf[0], TRANSMITTER);
    }
    if(buf[0] == 3){
        int bytes = llopen(buf[0], RECEIVER);
    }*/
    printf("2 bytes written\n");
}