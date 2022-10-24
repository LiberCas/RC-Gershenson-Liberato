#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int mopen(){
    printf("2/3: \n");
    unsigned char buf[3] = {0};

    fgets(buf, 3, stdin);
    
    if(buf[0] == '2'){
        int bytes = llopen(2, TRANSMITTER);
    }
    
    if(buf[0] == '3'){
        int bytes = llopen(3, RECEIVER);
    }
    else{
        printf("%c", buf[0]);
    }
}