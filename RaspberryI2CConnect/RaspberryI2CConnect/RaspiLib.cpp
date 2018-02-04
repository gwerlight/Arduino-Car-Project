#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include "RaspiLib.h"

void error(char *msg) {
	perror(msg);
	exit(0);
}


/* Initalisiere den Bus */
int getDevice(void)
{
	int device;
	unsigned long funcs;

	/* Geraetedatei oeffnen */
	printf("Opening device...");
	if ((device = open("/dev/i2c-1", O_RDWR)) < 0)
	{
		perror("open() failed");
		exit(1);
	}
	printf(" OK\n");

	/* Abfragen, ob die I2C-Funktionen da sind */
	if (ioctl(device, I2C_FUNCS, &funcs) < 0)
	{
		perror("ioctl() I2C_FUNCS failed");
		exit(1);
	}

	/* Ergebnis untersuchen */
	if (funcs & I2C_FUNC_I2C)
		printf("I2C\n");
	if (funcs & (I2C_FUNC_SMBUS_BYTE))
		printf("I2C_FUNC_SMBUS_BYTE\n");

	return device;
}

/* Suche nach I2C-Adressen */
void set_i2c_bus(int device, int aValue)
{
	int res;
	// Arduino auf Port 4
	int port = 4;

	if (ioctl(device, I2C_SLAVE, port) < 0)
		printf("ioctl() I2C_SLAVE failed\n");
	else
	{
		/* kann gelesen werden? */
		res = i2c_smbus_read_byte(device);
		if (res >= 0)
			printf("i2c chip found at: %x, val = %d\n", port, res);
			/* Kann geschrieben werden? */
			res = i2c_smbus_write_byte(device, aValue);
		if (res >= 0)
			printf("send to i2c chip at: %x, val = %d\n", port, aValue);
		/* Und die Antwort vom Arduino	*/
		res = i2c_smbus_read_byte(device);
		if (aValue > 50 && aValue < 230)
			while (res != aValue)
				res = i2c_smbus_read_byte(device);
		printf("i2c chip answer with: %x, val = %d\n", port, res);
	}
}

/* Suche nach I2C-Adressen */
void set_i2c_bus_text(int device, unsigned char aValue[16], unsigned char outStr[16])
{
	int res;
	// Arduino auf Port 4
	int port = 4;

	printf("i2c send to device : %x, text = %s\n", device, aValue);

	if (ioctl(device, I2C_SLAVE, port) < 0)
		printf("ioctl() I2C_SLAVE failed\n");
	else
	{
		/* kann gelesen werden? */
		res = i2c_smbus_read_byte(device);
		if (res >= 0)
			printf("i2c chip found at: %x, val = %d\n", port, res);
		/* Kann geschrieben werden? */
		res = i2c_smbus_write_block_data(device, 'T', 16, aValue);

		if (res >= 0)
			printf("send to i2c chip at: %x, val = %s\n", port, aValue);
		
		// 100 ms warten, damit der Raspi etwas ZEit hat für sich :)
		usleep(100000);

		/* Und die Antwort vom Arduino	*/
		unsigned char readValue[32];
		memset(readValue, 0, sizeof(readValue));
		// Geht aber halt sehr basic
		//res = read(device, readValue, 16);
		// Geht nicht, wohl ein Timingproblem
		//res = i2c_smbus_read_block_data(device, 'R', readValue);

		//res = i2c_smbus_read_i2c_block_data(device, '0', 8, readValue);
		//res = i2c_smbus_read_block_data(device, 'R', readValue);
		res = i2c_smbus_read_byte(device);


		// readvalue enthält an Stelle [0..x] die gesendeten Daten ( Länge Siehe [0] )
		memset(outStr, 0, sizeof(outStr));
		for (int y = 0; y < 16; y++) {
			if (readValue[y] > 31 && readValue[y] < 127)
				outStr[y] = readValue[y];
			else
				outStr[y] = 32;
		}
		outStr[15] = 0;
		if (res >= 0)
			printf("i2c chip answer with: %x, val = %s\n", port, outStr);
	}
}

/*
################################################
Konsolen Modus
################################################
*/
void changemode(int dir)
{
	static struct termios oldt, newt;

	if (dir == 1)
	{
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO | IEXTEN);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	}
	else
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int kbhit(void)
{
	struct timeval tv;
	fd_set rdfs;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO, &rdfs);

	select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &rdfs);

}

void qq	(int device)
{
	int value = 0;
	char ch;
	changemode(1);
	while (1)
	{
		while (!kbhit())
		{
			//putchar('.');
		}
		ch = getchar();

		if (ch == 'w' && value < 255)
		{
			value++;
			set_i2c_bus(device, value);
		}

		if (ch == 's'&& value > -1)
		{
			value--;
			set_i2c_bus(device, value);
		}

		if (ch == 'q')
			break;
	}
	changemode(0);
}

/*
##########################################
Debug Modus
##########################################
*/

void debugModus(void) {
	unsigned char sendbuffer[16];
	memset(sendbuffer, 0, sizeof(sendbuffer));
	unsigned char readbuffer[16];
	memset(readbuffer, 0, sizeof(readbuffer));
	int device = getDevice();

	int icount = 0;
	while (icount < 250) {
		sprintf((char*)sendbuffer, "%s-%03d-%03d\n", "F", icount, icount);
		set_i2c_bus_text(device, sendbuffer, readbuffer);
		icount = icount + 10;
	}

	while (icount > 0) {
		sprintf((char*)sendbuffer, "%s-%03d-%03d\n", "F", icount, icount);
		set_i2c_bus_text(device, sendbuffer, readbuffer);
		icount = icount - 10;
	}
}

/*
##########################################
WEB Modus
##########################################
*/

int func(int a) {
	return 2 * a;
}

int getData(int sockfd, int length, void* buffer) {
	int bytesRead = 0;
	int result = 0;
	while (bytesRead < length)
	{
		result = read(sockfd, (void*)((unsigned char*)buffer + bytesRead), length - bytesRead);
		if (result < 0)
		{	/* Read error. */
			perror("read");
			exit(EXIT_FAILURE);
		}

		bytesRead += result;
		*((unsigned char*)buffer + bytesRead) = 0x00;

		if (result == 0)
		{
			return -1;
		}
	}
	return bytesRead;
}

void sendData(int sockfd, char buffer[16]) {
	int n;

	sprintf(buffer, "%s\n", buffer);
	if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
		printf(const_cast<char *>("ERROR writing to socket"));
	buffer[n] = '\0';
}

void webModus(void)
{
	int sockfd, newsockfd, portno = 51717, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int data;
	unsigned char bufferread[16];
	unsigned char bufferwrite[16];

	printf("using port #%d\n", portno);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(const_cast<char *>("ERROR opening socket"));
	bzero((char *)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd,
		(struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		error(const_cast<char *>("ERROR on binding"));
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	//--- infinite wait on a connection ---
	while (1) {
		printf("waiting for new client...\n");
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen)) < 0)
			error(const_cast<char *>("ERROR on accept"));
		printf("opened new communication with client\n");
		while (1) {
			//---- wait for a number from client ---
			memset(bufferread, 0, sizeof(bufferread));
			memset(bufferwrite, 0, sizeof(bufferwrite));

			data = getData(newsockfd, 9, (void*)bufferread);
			printf("Empfangen(Web) : %s \n", bufferread);

			if (data < 0)
				break;
			
			set_i2c_bus_text(getDevice(), bufferread, bufferwrite);

			//--- send new data back ---
			printf("Gesendet (Web) : %s\n", bufferwrite);
			sendData(newsockfd, (char*)bufferwrite);
		}
		close(newsockfd);

		//--- if -2 sent by client, we can quit ---
		if (data == -2)
			break;
	}
}





