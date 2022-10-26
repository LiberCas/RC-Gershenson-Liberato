#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int mopen(){
    printf("2/3: \n");
    unsigned char buf[3];
    memset(buf, 0, 3);

    fgets(buf, 2, stdin);
    
    if(buf[0] == '2'){
        int fd = llopen(2, TRANSMITTER);
        if(fd == 0){
            return 0;
        }
        char buf2[20] = "hello world";
        llwrite(buf2, 11);
        sleep(1);
        llclose();
    }
    
    if(buf[0] == '3'){
        int fd = llopen(3, RECEIVER);
        if(fd == 0){
            return 0;
        }
        unsigned char buf3[20];
        memset(buf3, 0, 20);
        llread(buf3);
        sleep(1);
        llclose();
        printf("{%s}\n", buf3);
        
    }
    else{
        printf("%c", buf[0]);
    }
}