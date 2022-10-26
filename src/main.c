// Main file of the serial port project.
// NOTE: This file must not be changed.

#include <stdio.h>

#include "applicationLayer.h"

#define BAUDRATE 9600
#define N_TRIES 3
#define TIMEOUT 3

// Arguments:
//   $1: port number
//   $2: role (T/R)
//   $3: file name

int main(int argc, char** argv) {
    printf("2/3: \n");
    unsigned char buf[3];
    memset(buf, 0, 3);

    fgets(buf, 2, stdin);

    if (buf[0] == '2')
    {
        mopen(2, TRANSMITTER, "penguin.gif", BAUDRATE, MAX_SEND_SIZE, N_TRIES, TIMEOUT);
        //sendreceiveFile(2, TRANSMITTER, "penguin.gif", BAUDRATE, MAX_SEND_SIZE, N_TRIES, TIMEOUT);
    }

    if (buf[0] == '3')
    {
        mopen(3, RECEIVER, "penguin.gif", BAUDRATE, MAX_SEND_SIZE, N_TRIES, TIMEOUT);
        //sendreceiveFile(3, RECEIVER, "penguin.gif", BAUDRATE, MAX_SEND_SIZE, N_TRIES, TIMEOUT);
    }
	return 0;
}