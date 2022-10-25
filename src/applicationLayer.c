#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int mopen(){
    printf("7/8: \n");
    unsigned char buf[3] = {0};

    fgets(buf, 3, stdin);
    
    if(buf[0] == '7'){
        int fd = llopen(7, TRANSMITTER);
        char buf2[20] = "hello world";
        llwrite(buf2, 11);
    }
    
    if(buf[0] == '8'){
        int bytes = llopen(8, RECEIVER);
        char buf3 = malloc(20);
        llread(&buf3);
        printf("%s", buf3);
    }
    else{
        printf("%c", buf[0]);
    }
}