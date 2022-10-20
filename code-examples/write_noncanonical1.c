// Write to serial port in non-canonical mode
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256
#define CTRL_FRAME_SIZE 5
#define FLAG 0x7E
#define ADDRESS 0X03
#define CTRL_SET 0x03
#define CTRL_DISC 0x0B
#define CTRL_UA 0x07

int alarmEnabled = FALSE;
int alarmCount = 0;

volatile int STOP = FALSE;

void alarmHandler(int signal, int fd)
{
    alarmEnabled = FALSE;
    alarmCount++;
    sendSET(fd);
}

/*
int readResponse(int fd, char* ){
    unsigned char temp[1] = {0};
    unsigned char buf2[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char
    unsigned int i=0;
    while (STOP == FALSE)
    {
        read(fd, temp, 1);
        buf2[i] = temp[0];
        i++;
        if (temp[0] == '\0')
            STOP = TRUE;
    }
    printf(":%s:%d\n", buf2, i-1);
}


int beginCommunication(int fd)
{
    // Set alarm function handler
    (void)signal(SIGALRM, alarmHandler(fd));
    sendSET(fd);
    while (alarmCount < 4)
    {
        read(fd, temp, 1);
        if (alarmEnabled == FALSE)
        {
            alarm(3); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }
    }
    printf("Ending program\n");
    return 0;
}*/


int sendSET(int fd){
    unsigned char setFrame[CTRL_FRAME_SIZE] = {0};
    setFrame[0] = setFrame[4] = FLAG;
    setFrame[1] = ADDRESS;
    setFrame[2] = CTRL_SET;
    setFrame[3] = setFrame[1] ^ setFrame[2];
    int bytes = write(fd, setFrame, CTRL_FRAME_SIZE);
}

int main(int argc, char *argv[])
{
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];
    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
    }
    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }
    struct termios oldtio;
    struct termios newtio;
    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }
    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 5;  // Blocking read until 5 chars received
    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)
    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);
    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    printf("New termios structure set\n");

    int success = sendSET(fd);

    //Writing string
    printf("Please enter a string: \n");
    unsigned char buf[BUF_SIZE] = {0};
    fgets(buf, BUF_SIZE, stdin);
    int bytes = write(fd, buf, BUF_SIZE);
    printf("%ld bytes written\n", strlen(buf));
    sleep(1);


    //Reading response
    unsigned char temp[1] = {0};
    unsigned char buf2[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char
    unsigned int i=0;
    while (STOP == FALSE)
    {
        read(fd, temp, 1);
        buf2[i] = temp[0];
        i++;
        if (temp[0] == '\0')
            STOP = TRUE;
    }
    printf(":%s:%d\n", buf2, i-1);


    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    close(fd);
    return 0;
}
