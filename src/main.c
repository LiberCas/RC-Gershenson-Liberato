// Main file of the serial port project.
// NOTE: This file must not be changed.

#include <stdio.h>
#include <stdlib.h>

#include "applicationLayer.h"

#define BAUDRATE 9600
#define N_TRIES 3
#define TIMEOUT 4

// Arguments:
//   $1: /dev/ttySxx
//   $2: tx | rx
//   $3: filename
int main(int argc, char** argv) {
    if(argc != 4){
        printf("Incorrect program usage\n");
        return 1;
    }
    int port = atoi(argv[1]);
    int roleint = atoi(argv[2]);
    if(roleint != TRANSMITTER && roleint != RECEIVER){
        printf("Invalid Role\n");
        return 1;
    }
    LinkLayerRole role = roleint;
    char filename[MAX_FILE_NAME+1];
    memset(filename, 0, MAX_FILE_NAME+1);
    strcpy(filename, argv[3]);
    int success = sendreceiveFile(port, role, filename, BAUDRATE, MAX_PACKAGE_SIZE, N_TRIES, TIMEOUT);
	return success;
}
