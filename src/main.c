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
//   $2: filename (optional)
//   $3: tx | rx (optional -> receiver is default)
int main(int argc, char** argv) {
    if(argc < 2 || argc > 5){
        printf("Incorrect input, program usage:\nbin\\nserial port (filename) (role) (n_tries)");
        return 1;
    }
    //Get port
    int sizeport = strlen(argv[1]);
    char* port = malloc(sizeport+1);
    memset(port, 0, sizeport+1);
    memcpy(port, argv[1], sizeport);
    int roleint;
    int nTries;
    char filename[MAX_FILE_NAME+1];

    memset(filename, 0, MAX_FILE_NAME+1);
    strcpy(filename, argv[2]);

    if(argc < 4){
        roleint = RECEIVER;
    } else {
        roleint = atoi(argv[3]);
        if(roleint != TRANSMITTER && roleint != RECEIVER){
            printf("Invalid arg 3: Role\n'1' -> Receiver\n'0' -> Transmiter");
            free(port);
            return 1;
        }
    }
    
    if(argc > 4)
        nTries = = atoi(argv[4]);
    else
        nTries = N_TRIES;
    
    LinkLayerRole role = roleint;
    int success = sendreceiveFile(port, role, filename, BAUDRATE, MAX_DATA_SIZE, nTries, TIMEOUT);
    free(port);
	return success;
}
