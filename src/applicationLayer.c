#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int mopen(){
    printf("2/3: \n");
    unsigned char buf[3] = {0};

    fgets(buf, 3, stdin);
    
    if(buf[0] == '2'){
        int fd = llopen(2, TRANSMITTER);
        char buf2[20] = "hello world";
        llwrite(buf2, 11);
    }
    
    if(buf[0] == '3'){
        int bytes = llopen(3, RECEIVER);
        char buf3[20];
        llread(&buf3);
        printf("%s", buf3);
    }
    else{
        printf("%c", buf[0]);
    }
}