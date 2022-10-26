#include "applicationLayer.h"
#include <stdio.h>
#include <stdlib.h>

ApplicationLayer *al;

int sendreceiveFile(int door, LinkLayerRole role, char *file, int baudrate, int maxSendSize, int nRetries, int timeout)
{
    initApplicationLayer(role, file, baudrate, maxSendSize, nRetries, timeout);

    llopen(door, role);
    if (role == TRANSMITTER){
        sendFileLoop();
    }
    else if (role == RECEIVER){
        receiveFileLoop();
    }
    else{
        return 0;
    }
    sleep(1);
    return (llclose());
}

int initApplicationLayer(LinkLayerRole role, char *file, int baudrate, int maxSendSize, int nRetries, int timeout)
{
    al = (ApplicationLayer *)malloc(sizeof(ApplicationLayer));

    al->role = role;
    memcpy(al->filename, file, MAX_FILENAME_SIZE);

    if (!initLinkLayer(baudrate, nRetries, timeout))
    {
        printf("ERROR: Could not initialize Link Layer.\n");
        return 0;
    }
    saveOldTio();
    setNewTio();
    return 1;
}

int getFileSize(FILE* file) {
	// saving current position
	long int currentPosition = ftell(file);

	// seeking end of file
	if (fseek(file, 0, SEEK_END) == -1) {
		printf("ERROR: Could not get file size.\n");
		return -1;
	}

	// saving file size
	long int size = ftell(file);

	// seeking to the previously saved position
	fseek(file, 0, currentPosition);

	// returning size
	return size;
}

int sendFileLoop()
{
	FILE* file = fopen(al->filename, "rb");
	if (!file) {
		printf("ERROR: Could not open file to be sent.\n");
		return 0;
    }
	int fileSize = getFileSize(file);
	char fileSizeBuf[sizeof(int) * 3 + 2];
	snprintf(fileSizeBuf, sizeof fileSizeBuf, "%d", fileSize);

	
	if (!sendControlPackage(al->fd, CTRL_PKG_START, fileSizeBuf, al->filename))
		return 0;

	char* fileBuf = malloc(MAX_SEND_SIZE);

	// read file chunks
	unsigned int readBytes = 0, writtenBytes = 0, i = 0;
	while ((readBytes = fread(fileBuf, sizeof(char), MAX_SEND_SIZE, file)) > 0) {
		// send those chunks inside data packages
		if (!sendDataPackage(al->fd, (i++) % 255, fileBuf, readBytes)) {
			free(fileBuf);
			return 0;
		}

		// reset file buffer
		fileBuf = memset(fileBuf, 0, MAX_SEND_SIZE);

		// increment no. of written bytes
		writtenBytes += readBytes;
	}
	printf("\n\n");

	free(fileBuf);

	if (fclose(file) != 0) {
		printf("ERROR: Unable to close file.\n");
		return 0;
	}

	if (!sendControlPackage(al->fd, CTRL_PKG_END, "0", ""))
		return 0;

	printf("\n");
	printf("File successfully transferred.\n");

	return 1;
}

int receiveFileLoop() {
	int controlStart, fileSize;
	char* fileName;

	if (!receiveControlPackage(al->fd, &controlStart, &fileSize, &fileName))
		return 0;

	if (controlStart != CTRL_PKG_START) {
		printf(
				"ERROR: Control package received but its control field - %d - is not C_PKG_START",
				controlStart);
		return 0;
	}

	// create output file
	FILE* outputFile = fopen(al->filename, "wb");
	if (outputFile == NULL) {
		printf("ERROR: Could not create output file.\n");
		return 0;
	}

	printf("\n");
	printf("Created output file: %s\n", al->filename);
	printf("Expected file size: %d (bytes)\n", fileSize);
	printf("\n");

	int fileSizeReadSoFar = 0, N = -1;
	while (fileSizeReadSoFar != fileSize) {
		int lastN = N;
		char* fileBuf = NULL;
		int length = 0;

		// receive data package with chunk and put chunk in fileBuf
		if (!receiveDataPackage(al->fd, &N, &fileBuf, &length)) {
			printf("ERROR: Could not receive data package.\n");
			free(fileBuf);
			return 0;
		}

		if (N != 0 && lastN + 1 != N) {
			printf("ERROR: Received sequence no. was %d instead of %d.\n", N,
					lastN + 1);
			free(fileBuf);
			return 0;
		}

		// write received chunk to output file
		fwrite(fileBuf, sizeof(char), length, outputFile);
		free(fileBuf);

		// increment no. of read bytes
		fileSizeReadSoFar += length;

	}
	printf("\n\n");

	// close output file
	if (fclose(outputFile) != 0) {
		printf("ERROR: Closing output file.\n");
		return -1;
	}

	// receive end control package
	int controlPackageTypeReceived = -1;
	if (!receiveControlPackage(al->fd, &controlPackageTypeReceived, 0, NULL)) {
		printf("ERROR: Could not receive END control package.\n");
		return 0;
	}

	if (controlPackageTypeReceived != CTRL_PKG_END) {
		printf("ERROR: Control field received (%d) is not END.\n",
				controlPackageTypeReceived);
		return 0;
	}

	printf("\n");
	printf("File successfully received.\n");

	return 1;
}





int sendControlPackage(int fd, int C, char* fileSize, char* fileName) {

	// calculate control package size
	int packageSize = 5 + strlen(fileSize) + strlen(fileName);
	unsigned int i = 0, pos = 0;

	// create control package
	unsigned char controlPackage[packageSize];
	controlPackage[pos++] = C;
	controlPackage[pos++] = PARAM_FILE_SIZE;
	controlPackage[pos++] = strlen(fileSize);
	for (i = 0; i < strlen(fileSize); i++)
		controlPackage[pos++] = fileSize[i];
	controlPackage[pos++] = PARAM_FILE_NAME;
	controlPackage[pos++] = strlen(fileName);
	for (i = 0; i < strlen(fileName); i++)
		controlPackage[pos++] = fileName[i];

	if (C == CTRL_PKG_START) {
		printf("\n");
		printf("File: %s\n", fileName);
		printf("Size: %s (bytes)\n", fileSize);
		printf("\n");
	}

	// send control package
	if (!llwrite(controlPackage, packageSize)) {
		printf(
				"ERROR: Could not write to link layer while sending control package.\n");
		free(controlPackage);

		return 0;
	}
	return 1;
}

int receiveControlPackage(int fd, int* controlPackageType, int* fileLength,
		char** fileName) {
	// receive control package
	unsigned char* package;
	unsigned int totalSize = llread(package);
	if (totalSize < 0) {
		printf(
				"ERROR: Could not read from link layer while receiving control package.\n");
		return 0;
	}

	// process control package type
	*controlPackageType = package[0];

	unsigned int i = 0, numParams = 2, pos = 1, numOcts = 0;
	for (i = 0; i < numParams; i++) {
		int paramType = package[pos++];

		switch (paramType) {
		case PARAM_FILE_SIZE: {
			numOcts = (unsigned int) package[pos++];

			char* length = malloc(numOcts);
			memcpy(length, &package[pos], numOcts);

			*fileLength = atoi(length);
			free(length);

			break;
		}
		case PARAM_FILE_NAME:
			numOcts = (unsigned char) package[pos++];
			memcpy(*fileName, &package[pos], numOcts);

			break;
		}
	}

	return 1;
}

int sendDataPackage(int fd, int N, const char* buffer, int length) {
	unsigned char C = CTRL_PKG_DATA;
	unsigned char L2 = length / 256;
	unsigned char L1 = length % 256;

	// calculate package size
	unsigned int packageSize = 4 + length;

	// allocate space for package header and file chunk
	unsigned char* package = (unsigned char*) malloc(packageSize);

	// build package header
	package[0] = C;
	package[1] = N;
	package[2] = L2;
	package[3] = L1;

	// copy file chunk to package
	memcpy(&package[4], buffer, length);

	// write package
	if (!llwrite(package, packageSize)) {
		printf(
				"ERROR: Could not write to link layer while sending data package.\n");
		free(package);

		return 0;
	}

	free(package);

	return 1;
}

int receiveDataPackage(int fd, int* N, char** buf, int* length) {
	unsigned char* package;

	// read package from link layer
	unsigned int size = llread(package);
	if (size < 0) {
		printf(
				"ERROR: Could not read from link layer while receiving data package.\n");
		return 0;
	}

	int C = package[0];
	*N = (unsigned char) package[1];
	int L2 = package[2];
	int L1 = package[3];

	// assert package is a data package
	if (C != CTRL_PKG_DATA) {
		printf("ERROR: Received package is not a data package (C = %d).\n", C);
		return 0;
	}

	// calculate size of the file chunk contained in the read package
	*length = 256 * L2 + L1;

	// allocate space for that file chunk
	*buf = malloc(*length);

	// copy file chunk to the buffer
	memcpy(*buf, &package[4], *length);

	// destroy the received package
	free(package);

	return 1;
}










int mopen(int door, LinkLayerRole role, char* file, int baudrate, int maxSendSize, int nRetries, int timeout)
{

    if (door == 2)
    {
        initApplicationLayer(RECEIVER, file, baudrate, maxSendSize, nRetries, timeout);
        printf("\n %s : %d : %d, %s\n", __FUNCTION__, __LINE__, RECEIVER, file);
        int fd = llopen(2, TRANSMITTER);
        if (fd == 0)
        {
            return 0;
        }
        char buf2[20] = "hello world";
        llwrite(buf2, 11);
        sleep(1);
        llclose();
    }

    if (door == 3)
    {
        initApplicationLayer(RECEIVER, file, baudrate, maxSendSize, nRetries, timeout);
        int fd = llopen(3, RECEIVER);
        printf("\n %s : %d : %d, %s", __FUNCTION__, __LINE__, RECEIVER, file);
        if (fd == 0)
        {
            return 0;
        }
        unsigned char buf3[20];
        memset(buf3, 0, 20);
        printf("\n %s : %d : %d, %s", __FUNCTION__, __LINE__, RECEIVER, file);
        llread(buf3);
        printf("\n %s : %d : %d, %s", __FUNCTION__, __LINE__, RECEIVER, file);
        sleep(1);
        llclose();
        printf("{%s}\n", buf3);
    }
}