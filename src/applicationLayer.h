#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include "dataLinkLayer.h"

#define MAX_FILE_NAME 64
#define MAX_DATA_SIZE 119
#define PACKAGING_SIZE 4

typedef enum {
	C_DATA = 0x01, C_START = 0x02, C_END = 0x03
} ControlFieldPackage;

typedef enum {
	T_SIZE, T_NAME
} TPackage;

typedef struct  {
    int fd; 
    LinkLayerRole role;
    char filename[MAX_FILE_NAME+1];
} ApplicationLayer;

// Application layer main function.
// Arguments:
//  serialPort: Serial port name (e.g., /dev/ttyS0).
//  role: Link layer role {TRANSMITTER=0, RECEIVER=1}.
//  filename: Name of the file to send / receive.
//  baudRate: Baudrate of the serial port.
//  maxSendSize: Maximum bytes of file to send at once
//  nTries: Maximum number of frame retries.
//  timeout: Frame timeout.
int sendreceiveFile(char *port, LinkLayerRole role, const char *filename, int baudRate, int maxSendSize, int nTries, int timeout);
void initApplicationLayer(LinkLayerRole role, const char *filename, int baudRate, int maxSendSize, int nTries, int timeout);
int sendFileLoop();
int receiveFileLoop();
long int getFileSize(FILE* file);
int sendDataPackage(int n, const char* buffer, int length);
int sendControlPackage(ControlFieldPackage c, char *fileSizStr);
int receiveControlPackage(int * type, int * size);
int receiveDataPackage(int* N, char* fileBuf);
#endif // _APPLICATION_LAYER_H_