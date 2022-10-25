#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int mopen(){
    printf("7/8: \n");
    unsigned char buf[3];
    memset(buf, 0, 3);

    fgets(buf, 2, stdin);
    
    if(buf[0] == '7'){
        int fd = llopen(7, TRANSMITTER);
        char buf2[20] = "hello world";
        llwrite(buf2, 11);
    }
    
    if(buf[0] == '8'){
        int bytes = llopen(8, RECEIVER);
        unsigned char buf3[20];
        memset(buf3, 0, 20);
        llread(buf3);
        printf("{%s}\n", buf3);
    }
    else{
        printf("%c", buf[0]);
    }
}