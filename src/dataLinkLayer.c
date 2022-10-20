#include "dataLinkLayer.h"
#include <time.h>

int llopen(int door, ConnnectionMode mode) {
    if (mode == SENDER) {
		while (!connected) {
			if (try == 0 || alarmWentOff) {
				alarmWentOff = 0;

				if (try >= ll->numTries) {
					stopAlarm();
					printf("ERROR: Maximum number of retries exceeded.\n");
					printf("*** Connection aborted. ***\n");
					return 0;
				}

				sendCommand(al->fd, SET);

				if (++try == 1)
					setAlarm();
			}

			if (messageIsCommand(receiveMessage(al->fd), UA)) {
				connected = 1;

				printf("*** Successfully established a connection. ***\n");
			}
		}

		stopAlarm();
	}
	if (mode == RECEIVER) {
		while (!connected) {
			if (messageIsCommand(receiveMessage(al->fd), SET)) {
				sendCommand(al->fd, UA);
				connected = 1;

				printf("*** Successfully established a connection. ***\n");
			}
		}
	}
	return al->fd;
    return 0;
}


int llwrite(int fd, const unsigned char* buf, int length){
    return 0;
}

int llread(int fd, unsigned char* message){
    return 0;
}

int llclose(int fd){
    return 0;
}