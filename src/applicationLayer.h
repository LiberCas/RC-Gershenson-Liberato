#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include "dataLinkLayer.h"

#define MAX_FILE_NAME 64

typedef struct  {
    int fd; /*Descritor correspondente à porta série*/
    LinkLayerRole role;
    char filename[MAX_FILE_NAME+1];
} ApplicationLayer;


// Application layer main function.
// Arguments:
//   serialPort: Serial port name (e.g., /dev/ttyS0).
//   role: Application role {"tx", "rx"}.
//   baudrate: Baudrate of the serial port.
//   nTries: Maximum number of frame retries.
//   timeout: Frame timeout.
//   filename: Name of the file to send / receive.
int sendreceiveFile(int port, LinkLayerRole role, const char *filename, int baudRate, int maxSendSize, int nTries, int timeout);
int initApplicationLayer(LinkLayerRole role, const char *filename, int baudRate, int maxSendSize, int nTries, int timeout);
int sendFileLoop();
int receiveFileLoop();
long int getFileSize(FILE* file);

#endif // _APPLICATION_LAYER_H_