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

    
	if (sendControlPackage(C_START, fileSizeString) != 0)
		return 1;

	char* fileBuf = malloc(MAX_DATA_SIZE+1);
    memset(fileBuf, 0, MAX_DATA_SIZE+1);
    int fail = TRUE;

	unsigned int readBytes = 0, writtenBytes = 0, i = 0;
	while ((readBytes = fread(fileBuf, sizeof(char), MAX_DATA_SIZE, file)) > 0) {
        
		if (sendDataPackage((i++) % 255, fileBuf, readBytes) != 0) {
			free(fileBuf);
			return 1;
		}
        
		memset(fileBuf, 0, MAX_DATA_SIZE+1);
		writtenBytes += readBytes;
	}


	free(fileBuf);

	if (fclose(file) != 0) {
		printf("ERROR: Unable to close file.\n");
		return 1;
	}

	if (sendControlPackage(C_END, "0") != 0)
		return 1;

	printf("\nFile successfully transferred.\n");
	return 0;
}

int sendControlPackage(ControlFieldPackage c, char *fileSizStr){
	int packageSize = 5 + strlen(fileSizStr) + strlen(al->filename);
    if(packageSize > MAX_PACKAGE_SIZE){
        printf("ERROR: Package dimensions too big.\n");
		return 1;
    }

	unsigned int i = 0, pos = 0;

	unsigned char* controlPackage = (unsigned char*) malloc (packageSize);
    memset(controlPackage, 0, packageSize);
    
	controlPackage[pos++] = c;
	controlPackage[pos++] = T_SIZE;
	controlPackage[pos++] = strlen(fileSizStr);
	for (i = 0; i < strlen(fileSizStr); i++){
        controlPackage[pos++] = fileSizStr[i];
    }
	controlPackage[pos++] = T_NAME;
	controlPackage[pos++] = strlen(al->filename);
	for (i = 0; i < strlen(al->filename); i++){
		controlPackage[pos++] = al->filename[i];
    }
	if(c == C_START) {
		printf("\nFile: %s\n", al->filename);
		printf("Size: %s (bytes)\n", fileSizStr);
	}

	if (llwrite(controlPackage, packageSize) != 0) {
		printf("ERROR: Could not write to link layer while sending control package.\n");
		free(controlPackage);
		return 1;
	}
	return 0;
}

int sendDataPackage(int n, const char* buffer, int length) {
	ControlFieldPackage c = C_DATA;
	unsigned char l2 = length / 256;
	unsigned char l1 = length % 256;

	unsigned int packageSize = PACKAGING_SIZE + length;
	unsigned char* package = (unsigned char*) malloc(packageSize+1);
    memset(package, 0, packageSize+1);

	package[0] = c;
	package[1] = n;
	package[2] = l2;
	package[3] = l1;

	memcpy(&package[PACKAGING_SIZE], buffer, length);
    int fail = llwrite(package, packageSize);
	if (fail != 0) {
		printf("ERROR: Could not write data package.\n");
		free(package);
		return 1;
	}

	free(package);
	return 0;
}



int receiveFileLoop(){
    int controlStart, fileSize;
	char* fileName;

	if (receiveControlPackage(&controlStart, &fileSize) != 0){
        printf("ERROR: Could not receive START control package");
        return 1;
    }
		
	if (controlStart != C_START) {
		printf("ERROR: Control package received but its control field - %d - is not C_PKG_START", controlStart);
		return 1;
	}

	FILE* outputFile = fopen(al->filename, "wb");
	if (outputFile == NULL) {
		printf("ERROR: Could not create output file.\n");
		return 1;
	}


	printf("Created output file: %s\n", al->filename);
	printf("Expected file size: %d (bytes)\n", fileSize);
    unsigned int size;
    int fail = 0, N = -1;;
    int lastN = N;
	int fileSizeReadSoFar = 0;
    unsigned char* fileBuf = (unsigned char*) malloc(MAX_DATA_SIZE+1);
    memset(fileBuf, 0, MAX_DATA_SIZE+1);
    
	while (fileSizeReadSoFar < fileSize) {
		lastN = N;
        
        size = receiveDataPackage(&N, fileBuf);
		if (size == -1) {
			printf("ERROR: Could not receive data package.\n");
			free(fileBuf);
			return 1;
		}
        
		if (N != 0 && lastN + 1 != N) {
			printf("ERROR: Received sequence no. was %d instead of %d.\n", N, lastN + 1);
			free(fileBuf);
			return 1;
		}

		fwrite(fileBuf, sizeof(char), size, outputFile);
		fileSizeReadSoFar += size;
	}

	if (fclose(outputFile) != 0) {
		printf("ERROR: Couldn't close output file.\n");
		return 1;
	}

	int controlPackageTypeReceived = -1;
	if (receiveControlPackage(&controlPackageTypeReceived, &fileSize) != 0) {
		printf("ERROR: Could not receive END control package.\n");
		return 1;
	}

	if (controlPackageTypeReceived != C_END) {
		printf("ERROR: Control field received (%d) is not END.\n", controlPackageTypeReceived);
		return 1;
	}

	printf("File successfully received.\n");
	return 0;
}

int receiveControlPackage(int * type, int * size){
	unsigned char* package = (unsigned char*) malloc(MAX_PACKAGE_SIZE+1);
    memset(package, 0, MAX_PACKAGE_SIZE+1);

	unsigned int totalSize = llread(package);
	if (totalSize <= 0) {
		printf("ERROR: Could not read from link layer while receiving control package.\n");
		return 1;
	}

	*type = package[0];
    int paramType=0;
	unsigned int i = 0, numParams = 2, pos = 1, numOcts = 0;

	for (i = 0; i < numParams; i++) {
		paramType = package[pos++];
		switch (paramType) {
		case T_SIZE: {
			numOcts = (unsigned int) package[pos++];

			char* length = malloc(numOcts);
			memcpy(length, &package[pos], numOcts);
			pos += numOcts;
			*size = atoi(length);
			free(length);
			break;
		}
		case T_NAME:
			numOcts = (unsigned char) package[pos++];
            memset(al->filename, 0, MAX_FILE_NAME);
			memcpy(al->filename, &package[pos], numOcts);
			break;
		}
	}
	return 0;
}

int receiveDataPackage(int* N, char* fileBuf){
    unsigned char* package = (unsigned char*) malloc(MAX_PACKAGE_SIZE+1);
    memset(package, 0, MAX_PACKAGE_SIZE+1);

    int length =0;

	unsigned int size = llread(package);
	if (size <= 0) {
		printf("ERROR: Could not read from link layer while receiving data package.\n");
		return -1;
	}

	int C = package[0];
	*N = (unsigned char) package[1];
	int L2 = package[2];
	int L1 = package[3];

	if (C != C_DATA) {
		printf("ERROR: Received package is not a data package (C = %d).\n", C);
		return -1;
	}

	length = 256 * L2 + L1;
	memcpy(fileBuf, &package[PACKAGING_SIZE], length);

	free(package);
	return length;
}