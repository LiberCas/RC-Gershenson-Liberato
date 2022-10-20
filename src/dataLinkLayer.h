typedef enum {
	SENDER, RECEIVER
} ConnnectionMode;

#define FLAG 0x7E
#define ESCAPE = 0x7D
#define AW 0x03
#define AR 0x01
#define C 0x03

int llopen(int door, ConnnectionMode mode);
int llwrite(int fd, const unsigned char* buf, int length);
int llread(int fd, unsigned char* message);
int llclose(int fd);