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
	C_SET = 0x03, C_DISC = 0x0B, C_UA = 0x07, C_RR = 0x05, C_REJ = 0x01, C_I = 0X00
} ControlField;

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define COMMAND_FRAME_SIZE 5
#define BUF_SIZE 256
// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

typedef enum {
	TRANSMITTER, RECEIVER
} LinkLayerRole;

typedef enum {
	SET, DISC, UA, RR, REJ, I
} FrameType;

typedef struct {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
	LinkLayerRole role;
	int baudRate; /*Velocidade de transmissão*/
	unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
	unsigned int timeout; /*Valor do temporizador: 1 s*/
	unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
	char frame[BUF_SIZE]; /*Trama*/
	struct termios oldtio, newtio;
} LinkLayer;


// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(int door, LinkLayerRole role);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(const unsigned char* buf, int length);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(unsigned char* message);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int showStatistics);

int setPort(int door);
int saveOldTio();
int setNewTio();
int establishConnection();
char getA();
char getC(FrameType type);
int initLinkLayer(int door, LinkLayerRole role);
int createSFrame(FrameType type);
int sendFrame(int size);
int receiveFrame();
unsigned int frameType(FrameType type);
int stuff(unsigned char* frame, int sz);
#endif // _LINK_LAYER_H_