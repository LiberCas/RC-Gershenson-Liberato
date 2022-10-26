#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include "dataLinkLayer.h"

#define MAX_FILENAME_SIZE 256

typedef struct  {
    int fd; /*Descritor correspondente à porta série*/
    int role; /*TRANSMITTER | RECEIVER*/
    unsigned char filename[MAX_FILENAME_SIZE];
} ApplicationLayer;

typedef enum {
	CTRL_PKG_DATA = 1, CTRL_PKG_START = 2, CTRL_PKG_END = 3
} ControlPackageType;

typedef enum {
	PARAM_FILE_SIZE, PARAM_FILE_NAME
} ParameterType;

// Application layer main function.
// Arguments:
//   serialPort: Serial port name (e.g., /dev/ttyS0).
//   role: Application role {"tx", "rx"}.
//   baudrate: Baudrate of the serial port.
//   nTries: Maximum number of frame retries.
//   timeout: Frame timeout.
//   filename: Name of the file to send / receive.

int sendreceiveFile(int door, LinkLayerRole role, char* file, int baudrate, int maxSendSize, int nRetries, int timeout);
int initApplicationLayer(LinkLayerRole role, char* file, int baudrate, int maxSendSize, int nRetries, int timeout);
int sendFileLoop();
int receiveFileLoop();
int getFileSize(FILE* file);
int mopen(int door, LinkLayerRole role, char* file, int baudrate, int maxSendSize, int nRetries, int timeout);

#endif // _APPLICATION_LAYER_H_