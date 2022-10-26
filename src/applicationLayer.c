#include "applicationLayer.h"
#include "dataLinkLayer.h"
#include <stdio.h>

ApplicationLayer* al;

int sendreceiveFile(int port, LinkLayerRole role, const char *filename, int baudRate, int maxSendSize, int nTries, int timeout){
    initApplicationLayer(role, filename, baudRate, maxSendSize, nTries, timeout);

    int fail = llopen(port, role);
    if(fail == 1){
        return 1;
    }
    if (role == TRANSMITTER){
        fail = sendFileLoop();
    }
    else if (role == RECEIVER){
        fail = receiveFileLoop();
    }
    else{
        return 0;
    }
    if(fail == 1){
        return 1;
    }
    sleep(1);
    return (llclose());
}

int initApplicationLayer(LinkLayerRole role, const char *filename, int baudRate, int maxSendSize, int nTries, int timeout){
    al = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
    memset((char *)al, 0, sizeof(ApplicationLayer));
    al->role = role;
    memcpy(al->filename, filename, MAX_FILE_NAME);
    return 1;
}

long int getFileSize(FILE* file) {
	long int currentPosition = ftell(file);
	if (fseek(file, 0, SEEK_END) == -1) {
		printf("ERROR: Could not get file size.\n");
		return -1;
	}
	long int size = ftell(file);
	fseek(file, 0, currentPosition);
	return size;
}

int sendFileLoop(){
    FILE* file = fopen(al->filename, "rb");
	if (!file) {
		printf("ERROR: Could not open file to be sent.\n");
		return 1;
    }

	long int fileSize = getFileSize(file);
	char fileSizeString[sizeof(int) * 3 + 2];
	snprintf(fileSizeString, sizeof fileSizeString, "%d", fileSize);

	/*
	if (!sendControlPackage(al->fd, CTRL_PKG_START, fileSizeString, al->filename))
		return 0;
    */
	char* fileBuf = malloc(MAX_SEND_SIZE+1);
    memset(fileBuf, 0, MAX_SEND_SIZE+1);
    int fail = TRUE;

	unsigned int readBytes = 0, writtenBytes = 0, i = 0;
	while ((readBytes = fread(fileBuf, sizeof(char), MAX_SEND_SIZE, file)) > 0) {
        
		/*if (!sendDataPackage(al->fd, (i++) % 255, fileBuf, readBytes)) {
			free(fileBuf);
			return 0;
		}*/
        fail = llwrite(fileBuf, readBytes);
        if(fail == TRUE){
            printf("ERROR: Unable to send file.\n");
            free(fileBuf);
            return 1;
        }

		fileBuf = memset(fileBuf, 0, MAX_SEND_SIZE+1);
		writtenBytes += readBytes;
	}

	free(fileBuf);

	if (fclose(file) != 0) {
		printf("ERROR: Unable to close file.\n");
		return 1;
	}

    /*
	if (!sendControlPackage(al->fd, CTRL_PKG_END, "0", ""))
		return 0;*/

	printf("\nFile successfully transferred.\n");
	return 0;
}



int receiveFileLoop(){
    int controlStart, fileSize=946;
	char* fileName;

    /*
	if (!receiveControlPackage(al->fd, &controlStart, &fileSize, &fileName))
		return 0;

	if (controlStart != CTRL_PKG_START) {
		printf(
				"ERROR: Control package received but its control field - %d - is not C_PKG_START",
				controlStart);
		return 0;
	}*/

	// create output file
	FILE* outputFile = fopen(al->filename, "wb");
	if (outputFile == NULL) {
		printf("ERROR: Could not create output file.\n");
		return 1;
	}


	printf("Created output file: %s\n", al->filename);
    /*
	printf("Expected file size: %d (bytes)\n", fileSize);*/
    unsigned int size;
	int fileSizeReadSoFar = 0;// N = -1;
    char fileBuf[MAX_SEND_SIZE+1];
	while (fileSizeReadSoFar != fileSize) {
		//int lastN = N;

        memset(fileBuf, 0, MAX_SEND_SIZE+1);

        /*
		if (!receiveDataPackage(al->fd, &N, &fileBuf, &length)) {
			printf("ERROR: Could not receive data package.\n");
			free(fileBuf);
			return 0;
		}*/
        size = llread(fileBuf);
        /*
		if (N != 0 && lastN + 1 != N) {
			printf("ERROR: Received sequence no. was %d instead of %d.\n", N,
					lastN + 1);
			free(fileBuf);
			return 0;
		}*/

		fwrite(fileBuf, sizeof(char), size, outputFile);
		fileSizeReadSoFar += size;
	}

	if (fclose(outputFile) != 0) {
		printf("ERROR: Couldn't close output file.\n");
		return 1;
	}

/*
	int controlPackageTypeReceived = -1;
	if (!receiveControlPackage(al->fd, &controlPackageTypeReceived, 0, NULL)) {
		printf("ERROR: Could not receive END control package.\n");
		return 0;
	}

	if (controlPackageTypeReceived != CTRL_PKG_END) {
		printf("ERROR: Control field received (%d) is not END.\n",
				controlPackageTypeReceived);
		return 0;
	}*/

	printf("File successfully received.\n");
	return 0;
}