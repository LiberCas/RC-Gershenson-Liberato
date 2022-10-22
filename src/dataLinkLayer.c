#include "dataLinkLayer.h"
#include "applicationLayer.h"
#include "alarm.h"


LinkLayer* ll;
extern ApplicationLayer* al;
extern Alarm* alr;

int setPort(int door){
	char strDoor[3];
	itoa(door, strDoor, 10);
	char portPrelude[20] = "/dev/ttyS";
	strcat(portPrelude, strDoor);
	strcpy(ll->port, portPrelude);
	return 0;
}

int saveOldTio(){
	if (tcgetattr(al->fd, &ll->oldtio) == -1)
    {
        perror("Couldn't save old tio settings");
        exit(-1);
    }
	return 0;
}

int initLinkLayer(int door, LinkLayerRole role){
	setPort(door);
	ll->role = role;
	ll->baudRate = BAUDRATE;
	ll->sequenceNumber = 0;
	ll->timeout = 3;
	ll->numTransmissions = 3;
	memset(ll->frame, 0, BUF_SIZE);
	return 0;
}

int setNewTio(){
    memset(&ll->newtio, 0, sizeof(ll->newtio));

    ll->newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    ll->newtio.c_iflag = IGNPAR;
    ll->newtio.c_oflag = 0;

    
    ll->newtio.c_lflag = 0;
    ll->newtio.c_cc[VTIME] = 3; // Inter-character timer unused
    ll->newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    tcflush(al->fd , TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(al->fd , TCSANOW, &ll->newtio) == -1)
    {
        perror("Couldn't set new tio settings");
        exit(-1);
    }
    printf("New termios structure set\n");
	return 0;
}

char getA(){
	if(ll->role == TRANSMITTER)
		return AW;
	return AR;
}

char getC(FrameType type){
	char c;
	switch (type){
	case SET:
		c = C_SET;
		break;
	case DISC:
		c = C_DISC;
		break;
	case UA:
		c = C_UA;
		break;
	case RR:
		c = C_RR;
		break;
	case REJ:
		c = C_REJ;
		break;
	}
	return c;
}

int sendSFrame(FrameType type){
	memset(ll->frame, 0, BUF_SIZE);
	ll->frame[0] = FLAG;
	ll->frame[1] = getA();
	char c = getC(type);
	ll->frame[2] = c;
	if (c == C_REJ || c == C_RR)
		ll->frame[2] |= (ll->sequenceNumber << 7); //Ns
	ll->frame[3] = ll->frame[1] ^ ll->frame[2]; //BCC
	ll->frame[4] = FLAG;
	//call stuffing function here
	return sendFrame(COMMAND_FRAME_SIZE);
}

int sendFrame(int size){
	int bytes = write(al->fd, ll->frame, size);
	if (bytes != size){
		prinf("Couldn't write command");
		return -1;
	}
    printf("%d bytes written. Awaiting receiver response\n", bytes);
	return 0;
}

char* receiveFrame(){
	unsigned char temp[1] = {0};
	unsigned char buf[BUF_SIZE + 1] = {0};
	unsigned int i=0;
	unsigned int stop = FALSE;
	while (stop == FALSE){
		read(al->fd, temp, 1);
		buf[i] = temp[0];
		i++;
		if (temp[0] == '\0'){
			buf[i] = '\0';
			stop = TRUE;
		}   
	}
	return buf;
}

unsigned int frameType(char* frame, FrameType type){
	if(frame[2] == getC(type))
		return 1;
	return 0;
}

int establishConnection(){
	int connected = 0;
	resetAlarm();
	if (ll->role == TRANSMITTER) {
		while (!connected) {
			if (alr->alarmRang || alr->alarmCount == 0) {
				alr->alarmRang = 0;

				if (alr->alarmCount >= ll->numTransmissions) {
					printf("ERROR: Maximum number of retries exceeded. Connection aborted\n");
					return 0;
				}
				else{
					sendSFrame(SET);
					setAlarm(ll->timeout);
				}
			}

			if (frameType(receiveFrame(), UA)) {
				connected = 1;
				printf("Connection successfully established\n");
				return 0;
			}
		}
	}
	if (ll->role == RECEIVER) {
		while (!connected) {
			if (frameType(receiveFrame(), SET)) {
				sendSFrame(UA);
				connected = 1;

				printf("Connection successfully established\\n");
			}
		}
	}
}

int llopen(int door, LinkLayerRole role) {
	initLinkLayer(door, role);
	al->fd = open(ll->port, O_RDWR | O_NOCTTY);
	if (al->fd  < 0)
    {
        perror(ll->port);
        exit(-1);
    }
	saveOldTio();
	setNewTio();
	establishConnection();
	return al->fd;
}


int llwrite(const unsigned char* buf, int length){
    return 0;
}

int llread(unsigned char* message){
    return 0;
}

int llclose(int showStatistics){
    return 0;
}