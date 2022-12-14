#include "dataLinkLayer.h"
#include "applicationLayer.h"
#include "alarm.h"

LinkLayer *ll;
extern ApplicationLayer *al;
extern Alarm *alr;

int saveOldTio()
{
	if (tcgetattr(al->fd, &ll->oldtio) == -1)
	{
		perror("Couldn't save old tio settings");
		exit(-1);
	}
	return 0;
}

int initLinkLayer(char* door, LinkLayerRole role)
{
	ll = (LinkLayer *)malloc(sizeof(LinkLayer));
	strcpy(ll->port, door);
	ll->role = role;
	ll->baudRate = BAUDRATE;
	ll->sequenceNumber = 0;
	ll->timeout = 3;
	ll->numTransmissions = 1;
	memset(ll->sent_frame, 0, MAX_STUFFED_SIZE);
	memset(ll->received_frame, 0, MAX_UNSTUFFED_SIZE + FLAG_SIZE);
	return 0;
}

int setNewTio()
{
	memset(&ll->newtio, 0, sizeof(ll->newtio));

	ll->newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	ll->newtio.c_iflag = IGNPAR;
	ll->newtio.c_oflag = 0;

	ll->newtio.c_lflag = 0;
	ll->newtio.c_cc[VTIME] = 3; // Inter-character timer unused
	ll->newtio.c_cc[VMIN] = 0;	// Blocking read until 5 chars received

	// VTIME e VMIN should be changed in order to protect with a
	// timeout the reception of the following character(s)

	tcflush(al->fd, TCIOFLUSH);

	// Set new port settings
	if (tcsetattr(al->fd, TCSANOW, &ll->newtio) == -1)
	{
		perror("Couldn't set new tio settings");
		exit(-1);
	}
	printf("New termios structure set\n");
	return 0;
}

char getA(FrameType type, unsigned int analysingMessage)
{
	if ((ll->role == TRANSMITTER && analysingMessage == FALSE) || (ll->role == RECEIVER && analysingMessage == TRUE))
	{
		if ((type == I) || (type == SET) || (type == DISC))
		{
			return AW;
		}
		return AR;
	}
	if ((type == I) || (type == SET) || (type == DISC))
	{
		return AR;
	}
	return AW;
}

char getC(FrameType type)
{
	char c = 0;
	switch (type)
	{
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
	// case I
	return c;
}

int createSFrame(FrameType type)
{
	unsigned char frame[COMMAND_FRAME_SIZE-FLAG_SIZE +1];
	memset(frame, 0, COMMAND_FRAME_SIZE-FLAG_SIZE +1);
	frame[0] = FLAG;
	frame[0] = getA(type, FALSE);
	char c = getC(type);
	frame[1] = c;
	if (c == C_REJ || c == C_RR)
		frame[1] = frame[1] | (ll->sequenceNumber << 7); // Ns
	frame[2] = (frame[0] ^ frame[1]);					 // BCC
	return stuff(frame, COMMAND_FRAME_SIZE-FLAG_SIZE);
}

int stuff(unsigned char *frame, int sz)
{
	unsigned char stuffed[(sz * 2) + FLAG_SIZE];
	memset(stuffed, 0, (sz * 2) + FLAG_SIZE);
	unsigned int i = 0;
	unsigned int j = 1;
	stuffed[0] = FLAG;
	while (i < sz)
	{
		if ((frame[i] == ESCAPE) || (frame[i] == FLAG))
		{
			stuffed[j] = ESCAPE;
			j++;
		}
		stuffed[j] = frame[i];
		i++;
		j++;
	}
	stuffed[j] = FLAG;
	j++;
	memset(ll->sent_frame, 0, MAX_STUFFED_SIZE);
	memcpy(ll->sent_frame, stuffed, j);
	return j;
}

int sendFrame(int size)
{
	int bytes = write(al->fd, ll->sent_frame, size);
	if (bytes != size)
	{
		printf("WRITE ERROR");
		return -1;
	}
	return 0;
}

int receiveFrame()
{ // Also does destuffing for memory efficiency
	unsigned char temp[1] = {0};
	memset(ll->received_frame, 0, MAX_UNSTUFFED_SIZE + FLAG_SIZE);
	unsigned int i = 0;
	unsigned int stop = FALSE;
	unsigned int escaped = FALSE;
	unsigned int readSmth = FALSE;
	while (stop == FALSE)
	{
		readSmth = read(al->fd, temp, 1);
		if (i == 0 && readSmth == 0)
		{
			return 0;
		}
		if (temp[0] == FLAG)
		{
			if (i == 0)
			{
				// do nothing
			}
			else if (escaped)
			{
				escaped = FALSE;
			}
			else
			{
				stop = TRUE;
			}
		}
		if (temp[0] == ESCAPE)
		{
			if (escaped)
			{
				escaped = FALSE;
			}
			else
			{
				escaped = TRUE;
				continue;
			}
		}
		ll->received_frame[i] = temp[0];
		i++;
	}
	return i;
}

FrameType receivedFrameType()
{
	FrameType t = U;
	switch (ll->received_frame[2])
	{
	case C_SET:
		t = SET;
		break;
	case C_DISC:
		t = DISC;
		break;
	case C_UA:
		t = UA;
		break;
	case C_RR:
		t = RR;
		break;
	case C_RR1:
		t = RR;
		break;
	case C_REJ1:
		t = REJ;
		break;
	case C_REJ:
		t = REJ;
		break;
	case C_I0:
		t = I;
	case C_I1:
		t = I;
	}
	return t;
}

int establishConnection()
{
	unsigned int connected = FALSE;
	alr = (Alarm *)malloc(sizeof(Alarm));
	resetAlarm();
	if (ll->role == TRANSMITTER){
		while (!connected)
		{
			if (alr->alarmRang || alr->alarmCount == 0)
			{
				alr->alarmRang = FALSE;

				if (alr->alarmCount >= ll->numTransmissions)
				{
					stopAlarm();
					printf("ERROR: Maximum number of retries exceeded. Connection aborted\n");
					return 1;
				}
				else
				{
					int sz = createSFrame(SET);
					sendFrame(sz);
					setAlarm(ll->timeout);
				}
			}

			if ((receiveFrame() != 0) && (receivedFrameType() == UA))
			{
				connected = TRUE;
				stopAlarm();
				printf("Connection successfully established\n");
				return 0;
			}
		}
	}
	if (ll->role == RECEIVER)
	{
		while (!connected)
		{
			if ((receiveFrame() != 0) && (receivedFrameType() == SET))
			{
				int sz = createSFrame(UA);
				sendFrame(sz);
				stopAlarm();
				connected = TRUE;
				printf("Connection successfully established\n");
				return 0;
			}
			if (alr->alarmRang || alr->alarmCount == 0) {
				alr->alarmRang = FALSE;

				if (alr->alarmCount >= ll->numTransmissions)
				{
					stopAlarm();
					printf("ERROR: Maximum number of retries exceeded. Connection aborted\n");
					return 1;
				}
				else
				{
					setAlarm(ll->timeout);
				}
			}
		}
	}
	return 0;
}

int llopen(char* door, LinkLayerRole role)
{
	initLinkLayer(door, role);
	al->fd = open(ll->port, O_RDWR | O_NOCTTY);
	if (al->fd < 0)
	{
		perror(ll->port);
		return 1;
	}
	saveOldTio();
	setNewTio();
	int fail = establishConnection();
	if(fail == 0){
		return al->fd;
	}
	return 1;
}

int llwrite(const unsigned char *buf, int length)
{
	unsigned int transferring = TRUE;
	unsigned int newframe = TRUE;
	resetAlarm();
	int sz = 0;
	while (transferring)
	{
		if (alr->alarmRang || alr->alarmCount == 0)
		{
			alr->alarmRang = FALSE;

			if (alr->alarmCount >= ll->numTransmissions)
			{
				stopAlarm();
				printf("TIMEOUT ERROR\n");
				return 1;
			}

			else
			{
				if (newframe)
				{
					sz = createIFrame(buf, length);
					newframe = FALSE;
                    printf("Sending new frame\n");
				}
                else{
                    printf("Resending frame\n");
                }
				sendFrame(sz);
				setAlarm(ll->timeout);
			}
		}
		if (receiveFrame() != 0)
		{
			if (receivedFrameType() == RR)
			{
				if (receivedSFrameSN() != ll->sequenceNumber)
				{
					continue;
				}
				transferring = FALSE;
				printf("Received ACKNOWLEDGE, successfully sent frame\n");
			}
			else if (receivedFrameType() == REJ)
			{
                printf("Received REJECT, resending frame\n");
				stopAlarm();
				resetAlarm();
			}
		}
	}
	stopAlarm();
	return 0;
}

unsigned int createIFrame(const unsigned char *buf, int length)
{
	if (ll->sequenceNumber == 0)
	{
		ll->sequenceNumber = 1;
	}
	else
	{
		ll->sequenceNumber = 0;
	}
	unsigned char frame[length + I_FRAME_SIZE - FLAG_SIZE];
	memset(frame, 0, length + I_FRAME_SIZE - FLAG_SIZE);
	int i0 = 0;
	frame[i0++] = getA(I, FALSE);
	frame[i0++] = (ll->sequenceNumber << 6); // Ns
	frame[i0++] = (frame[0] ^ frame[1]);	  // BCC1
	int i;
	for (i = 0; i < length; i++)
	{
		frame[i + i0] = buf[i];
	}
	frame[i + i0] = makeBCC2(buf, length);
	return stuff(frame, length + I_FRAME_SIZE - FLAG_SIZE);
}

unsigned int receivedSFrameSN()
{
	return (ll->received_frame[2] >> 7);
}

unsigned int receivedIFrameSN()
{
	return (ll->received_frame[2] >> 6);
}

unsigned char makeBCC2(const unsigned char *buf, int size)
{
	unsigned char bcc = '\0';
	for (int i = 0; i < size; i++)
	{
		bcc = bcc ^ buf[i];
	}
	return bcc;
}

unsigned int llread(unsigned char *message)
{
	unsigned int transferring = TRUE;
	int receivedSize;
    resetAlarm();
    setAlarm(ll->timeout);
	while (transferring)
	{

        if (alr->alarmRang || alr->alarmCount == 0)
		{
			alr->alarmRang = FALSE;

			if (alr->alarmCount >= ll->numTransmissions)
			{
				stopAlarm();
				printf("TIMEOUT ERROR\n");
				return 1;
			}

			else
			{
				setAlarm(ll->timeout);
			}
		}
		receivedSize = receiveFrame();
		if (receivedSize != 0)
		{
			if (receivedFrameType() == SET) //Getting old sets
			{
				continue;
			}
			int success = analyzeReceivedFrame(receivedSize);
			int sendSize = 0;
			if (success == TRUE)
			{
                stopAlarm();
				if (receivedIFrameSN() == ll->sequenceNumber)
				{
					sendSize = createSFrame(RR);
				}
				else
				{
					ll->sequenceNumber = receivedIFrameSN();
					transferring = FALSE;
					sendSize = createSFrame(RR);
					printf("Frame received, sent ACKNOWLEDGE\n");
				}
			}
			else
			{
				sendSize = createSFrame(REJ);
				printf("Errors in frame, sent REJECT\n");
			}
			sendFrame(sendSize);
		}
	}
	memcpy(message, ll->received_frame+4, receivedSize - I_FRAME_SIZE);
	return (receivedSize - I_FRAME_SIZE);
}

unsigned int analyzeReceivedFrame(const unsigned int sz)
{
	if ((ll->received_frame[0] != FLAG) || (ll->received_frame[sz - 1] != FLAG))
	{ // FLAG error
		return FALSE;
	}
	if (receivedFrameType() == U)
	{ // Frame type error
		return FALSE;
	}
	if (ll->received_frame[1] != getA(receivedFrameType(), TRUE))
	{ // A error
		return FALSE;
	}
	if (ll->received_frame[3] != (ll->received_frame[1] ^ ll->received_frame[2]))
	{ // BCC1 error
		return FALSE;
	}
	if (receivedFrameType() == I)
	{ // BCC2 error
		if (ll->received_frame[4] == FLAG)
			return FALSE;
		if (makeBCC2(&ll->received_frame[4], sz - I_FRAME_SIZE) != ll->received_frame[sz - 2])
			return FALSE;
	}
	return TRUE;
}

int llclose() {
	printf("Attempting to terminate connection\n");
	unsigned int connected = TRUE;
	unsigned int success = FALSE;
	resetAlarm();

	if (ll->role == TRANSMITTER) {
		while (connected) {
			if (alr->alarmRang || alr->alarmCount == 0) {

				alr->alarmRang = FALSE;

				if (alr->alarmCount >= ll->numTransmissions) {
					stopAlarm();
					printf("ERROR: Maximum number of retries exceeded. Connection aborted\n");
					connected = FALSE;
					success = FALSE;
				}
				else{
					int sz = createSFrame(DISC);
					sendFrame(sz);
					setAlarm(ll->timeout);
				}
			}
			if ((receiveFrame() != 0) && (receivedFrameType() == DISC)) {
				stopAlarm();
				int sz = createSFrame(UA);
				sendFrame(sz);
				printf("Connection terminated\n");
				connected = FALSE;
				success = TRUE;
			}
		}
	}
	if (ll->role == RECEIVER) {
		while (connected){
			if (alr->alarmRang || alr->alarmCount == 0) {
				alr->alarmRang = FALSE;

				if (alr->alarmCount >= ll->numTransmissions){
					stopAlarm();
					printf("ERROR: Maximum number of retries exceeded. Connection aborted\n");
					connected = FALSE;
					success = FALSE;
				}
				else{
					setAlarm(ll->timeout);
				}
			}
			if ((receiveFrame() != 0) && (receivedFrameType() == DISC)){
				int sz = createSFrame(DISC);
				sendFrame(sz);
				resetAlarm();
				
				while(connected){ //Wait for UA
					if ((receiveFrame() != 0) && (receivedFrameType() == UA)){
						stopAlarm();
						printf("Connection terminated\n");
						connected = FALSE;
						success = TRUE;
					}
					if (alr->alarmRang || alr->alarmCount == 0) {
						alr->alarmRang = FALSE;

						if (alr->alarmCount >= ll->numTransmissions){
							stopAlarm();
							printf("ERROR: Maximum number of retries exceeded. Connection aborted\n");
							connected = FALSE;
							success = FALSE;
						}
						else{
							setAlarm(ll->timeout);
						}
					}
				}
			}
		}
	}
	if (tcsetattr(al->fd, TCSANOW, &ll->oldtio) == -1){
        perror("tcsetattr");
        exit(-1);
    }
    close(al->fd);
	free(ll);
	free(al);
	free(alr);
	return success;
}
