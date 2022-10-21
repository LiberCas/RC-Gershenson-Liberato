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

int establishConnection(LinkLayerRole role){
	int connected = 0;
	resetAlarm();
	if (role == TRANSMITTER) {
		while (!connected) {
			if (alr->alarmRang || alr->alarmCount == 0) {
				alr->alarmRang = 0;

				if (alr->alarmCount >= ll->numTransmissions) {
					printf("ERROR: Maximum number of retries exceeded.\n");
					printf("*** Connection aborted. ***\n");
					return 0;
				}
				else{
					sendCommand(al->fd, SET);
					setAlarm(ll->timeout);
				}
			}

			if (messageIsCommand(receiveMessage(al->fd), UA)) {
				connected = 1;
				printf("*** Successfully established a connection. ***\n");
			}
		}
	}
	if (role == RECEIVER) {
		while (!connected) {
			if (messageIsCommand(receiveMessage(al->fd), SET)) {
				sendCommand(al->fd, UA);
				connected = 1;

				printf("*** Successfully established a connection. ***\n");
			}
		}
	}
}

int llopen(int door, LinkLayerRole role) {
	setPort(door);
	al->fd = open(ll->port, O_RDWR | O_NOCTTY);
	if (al->fd  < 0)
    {
        perror(ll->port);
        exit(-1);
    }
	saveOldTio();
	setNewTio();
	ll->role = role;
	establishConnection(role);
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