#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include <termios.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define O_RDWR 02
#define O_NOCTTY 0400

#define FLAG 0x7E
#define ESCAPE 0x7D

#define AW 0x03
#define AR 0x01

typedef enum {
	C_SET = 0x03, C_DISC = 0x0B, C_UA = 0x07, C_RR = 0x05, C_REJ = 0x01, C_I0 = 0x00, C_I1 = 0x40, C_RR1 = 0x85, C_REJ1 = 0x81
} ControlField;

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define COMMAND_FRAME_SIZE 5
#define I_FRAME_SIZE 6
#define FLAG_SIZE 2
#define MAX_PACKAGE_SIZE 123 
#define MAX_UNSTUFFED_SIZE 127
#define MAX_STUFFED_SIZE 256

typedef enum {
	TRANSMITTER, RECEIVER
} LinkLayerRole;

typedef enum {
	SET, DISC, UA, RR, REJ, I, U
} FrameType;

typedef struct {
	char port[40]; /*Dispositivo /dev/ttySx, x = 0, 1*/
	LinkLayerRole role;
	int baudRate; /*Velocidade de transmissão*/
	unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
	unsigned int timeout; /*Valor do temporizador: 1 s*/
	unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
	unsigned char sent_frame[MAX_STUFFED_SIZE]; /*Trama*/
	unsigned char received_frame[MAX_UNSTUFFED_SIZE + FLAG_SIZE];
	struct termios oldtio, newtio;
} LinkLayer;


// Open a connection using the "port" parameters defined in struct linkLayer.
// Parameters other than door and role to be used in possible future customizations.
// Return "1" on success or "-1" on error.
int llopen(char* door, LinkLayerRole role, int baudRate, int maxSendSize, int nTries, int timeout);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(const unsigned char* buf, int length);

int llclose();

int saveOldTio();
int setNewTio();
int establishConnection();
char getC(FrameType type);
char getA(FrameType type, unsigned int analysingMessage);
int initLinkLayer(char* door, LinkLayerRole role, int baudRate, int maxSendSize, int nTries, int timeout);
int createSFrame(FrameType type);
int sendFrame(int size);
int receiveFrame();
FrameType receivedFrameType();
unsigned int createIFrame(const unsigned char* buf, int length);
unsigned int receivedSFrameSN();
unsigned char makeBCC2(const unsigned char* buf, int size);
int stuff(unsigned char* frame, int sz);
unsigned int receivedIFrameSN();
unsigned int analyzeReceivedFrame(const unsigned int sz);
unsigned int llread(unsigned char* message);

#endif // _LINK_LAYER_H_
