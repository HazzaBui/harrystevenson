//Set up colour array, format [] byte byte byte rgb

#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>

#include "lzfP.h"

//Include graphics stuff
#include <linux/fb.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>

//Include network stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

//Include system access stuff
#include "keyboardState.h"


#define WIDTH 640
#define HEIGHT 480
#define SAMPLESIZEX 20
#define SAMPLESIZEY 20
#define SAMPLETHRESHOLD 5000
#define SAMPLECHECKRANGE 10
#define TRANSMISSIONSIZE 484 //- 28 bytes ip/udp header 
#define STREAMHEADEROFFSET 4
#define LOOPTIMEMS 500
#define FRAMESENDTIMER 600

//Signatures
void createVectors(int start, int starty, int sampleSizeX, int sampleSizeY);
void serialiseData();
void flushData();
void setupConnection();
void sendKeyframe();
receiveType checkIncomingStream();
void initialiseVars();
void cleanup();
void parseArgs(int argc, char* argv[]);
receiveType translateReceiveType(char c);
void sendCharStream(char *stream, int32_t streamSize, char sendType);
int vectorImageDiff();

//Application vars
int fbfd;
char *fbp;
struct fb_fix_screeninfo sinfo;
char* prevImage;
char* curImage;
moveVector** moveVecs;
int numOfSamples;
int vectorCount;
streamHeader* sh;
FILE *debuglog;
long int framebuffersize;

//Network vars
int portInt;
char* portChar;
int status;
struct addrinfo host_info;
struct addrinfo *host_info_list;
int socketDesc;
struct sockaddr_in returnSocketDesc;
pollfd pfd;
char* incomingBuffer;
short incomingBufferLength;
char* sendBuffer;
unsigned int sendBufferLength;
char* sendTypeBuffer;
unsigned int sendTypeBufferLength;
size_t sendSize;


int main(int argc, char* argv[])
{
	struct timespec loopTimer;
	long timeElapsed = 0;
	long sleepTime = 0;

	fprintf(stdout, "\nStart");
	fflush(stdout);
	receiveType rt;
	bool loop = true;
	
	parseArgs(argc, argv);
	initialiseVars();
	setupConnection();


	fprintf(stdout, "\nBeginning loop ");
fflush(stdout);

	//Use 'loop' to maintain gameloop
	//Perform all compression/communication here
	fprintf(debuglog, "Entering Loop\n");
	while(loop)
	{
		clock_gettime(CLOCK_MONOTONIC, &loopTimer);
		timeElapsed = loopTimer.tv_nsec;

		rt = checkIncomingStream();

		switch(rt)
		{
			case KEYFRAMESEND:
				sendKeyframe();
				sleepTime = 0;
				//Send keyframe
			break;
			case CLOSECONNECTION:
				//close down connection and cleanup
				fprintf(stdout, "\nClose Connection Received\n");
				loop = false;
			break;
			case NEWKEYSTATE:
				changeState(incomingBuffer+3, incomingBufferLength - 3);
			break;
		}

		//Send keyframe every time timer reaches send threshold
		if(sleepTime > FRAMESENDTIMER)
		{
			sendKeyframe();
			sleepTime -= FRAMESENDTIMER;
		}
		else
		{
			//Else send p frame
		//	vectorImageDiff();
		}

		//Update send timer
		clock_gettime(CLOCK_MONOTONIC, &loopTimer);
		timeElapsed = loopTimer.tv_nsec - timeElapsed;
		timeElapsed = timeElapsed < 0 ? timeElapsed += 1000000000 : timeElapsed;
		sleepTime += timeElapsed / 1000000;

		//flush log to ensure crash outs still record data
		fflush(debuglog);

	}
	fprintf(debuglog, "Ending loop, closing down\n");
	
	//cleanup, run any post process, inform game application etc

	cleanup();
}

void initialiseVars()
{
	debuglog = fopen("/var/log/fyp/debug.log", "w");
	chown("/var/log/fyp/debug.log", 1000, 0);
	chmod("/var/log/fyp/debug.log", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	fprintf(debuglog, "Initialise Vars\n");
	fflush(debuglog);
	
	fbfd = open("/dev/fb0", O_RDWR);
	ioctl(fbfd, FBIOGET_FSCREENINFO, &sinfo);
	framebuffersize = sinfo.smem_len;
	fbp = (char*)mmap(0, framebuffersize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);


	sh = new streamHeader(WIDTH, HEIGHT, SAMPLESIZEX, SAMPLESIZEY);


	numOfSamples = (WIDTH / SAMPLESIZEX) * (HEIGHT / SAMPLESIZEY);

	fprintf(stdout, "\nInitial alloc");
	fflush(stdout);

	//allocate mem for current and previous image
	prevImage = (char*)malloc(sizeof(char) * WIDTH * HEIGHT * 3);
	curImage = (char*)malloc(sizeof(char) * WIDTH * HEIGHT * 3);


	memcpy(fbp, curImage, framebuffersize);

	bool tempor = false;

	//Movement Vector list
	moveVecs = new moveVector*[numOfSamples];
	fprintf(stdout, "\nCreate %d move vectors");

	for(int i = 0; i < numOfSamples; i++)
	{
		moveVecs[i] = new moveVector();
	}
	fprintf(stdout, "\nCreated");

	//Network receive buffer
	incomingBuffer = (char*)malloc(sizeof(char) * TRANSMISSIONSIZE);
	incomingBufferLength = 0;
	
	//Add 2 initial bytes for UDP packet pos, then advance pointer 2
	sendBuffer = (char*)malloc((sizeof(char)* WIDTH * HEIGHT * 3) + sizeof(char)*STREAMHEADEROFFSET);  
	sendBuffer += STREAMHEADEROFFSET;
	sendBufferLength = WIDTH * HEIGHT * 3;

	sendTypeBuffer = (char*)malloc(sizeof(char)* 40);
	sendTypeBufferLength = 40;

	initialiseKeyboard();

	fprintf(stdout, "\nFinished initialise");
	fflush(stdout);
}

void cleanup()
{
	fclose(debuglog);
}


void serialiseData()
{
	
	for(int i = 0; i < vectorCount; i++)
	{
		//push each move vector
	}
}

void setupConnection()
{
	fprintf(stdout, "\nUsing ip %d and port %d ", 0, portInt);
	fprintf(debuglog, "Using port: %d\n", portInt);
	//Create connection
	//send stream header - sh*
	int optval = 1;

	memset(&host_info, 0, sizeof(addrinfo));
	host_info.ai_family = AF_UNSPEC;
	host_info.ai_socktype = SOCK_DGRAM;
	host_info.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, portChar, &host_info, &host_info_list);

	socketDesc = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);

	status = setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof(int));
	status = bind(socketDesc, host_info_list->ai_addr, host_info_list->ai_addrlen);

	pfd.fd = socketDesc;
	pfd.events = POLLIN;

	fprintf(stdout, "\nBind result: %d \n", status);
	fflush(stdout);

	fprintf(debuglog, "Network bound\n");

	socklen_t socklen = (socklen_t)sizeof(sockaddr);

	recvfrom(socketDesc, incomingBuffer, TRANSMISSIONSIZE, 0, (sockaddr*)&returnSocketDesc, &socklen); 

	fprintf(debuglog, "Client connected\n");

	//Respond to ok the 'connection'
	sendTypeBuffer[0] = (char)CONNECTOK;
	sendto(socketDesc, sendTypeBuffer, 1, 0, (sockaddr*)&returnSocketDesc, sizeof(sockaddr_in));

	//Send stream header
	sprintf(sendTypeBuffer, "%d\n%d\n%d\n%d\n%d\n", sh->imageSizeX, sh->imageSizeY, sh->sampleSizeX, sh->sampleSizeY, TRANSMISSIONSIZE);
	fprintf(stdout, "\nSending stream header: %s", sendTypeBuffer);
		sendto(socketDesc, sendTypeBuffer, sendTypeBufferLength, 0, (sockaddr*)&returnSocketDesc, sizeof(sockaddr_in));
	
	
	
	fprintf(stdout, "\nClient connected ");

	fprintf(debuglog, "Successfully returned\n");
}


void sendKeyframe()
{
	memcpy(curImage, prevImage, framebuffersize);
	memcpy(fbp, curImage, framebuffersize);

	fprintf(debuglog, "Image size: %d sendBufferLength: %d\n",  WIDTH * HEIGHT * 3, sendBufferLength);
	fflush(debuglog);

	unsigned int length = lzf_compress(curImage, WIDTH * HEIGHT * 3, sendBuffer, sendBufferLength); 

	if(length > 0)
	{
		sendCharStream(sendBuffer, length, (char)KEYFRAME);
	}

}

//Check the stream for connection close, resend etc
receiveType checkIncomingStream()
{
	receiveType rt = EMPTY;

	if(poll(&pfd, 1, 1) != 0)
	{
		int size = 	recv(socketDesc, incomingBuffer, TRANSMISSIONSIZE, 0);
		fflush(stdout);

		if(size > 0)
		{
			rt = translateReceiveType(incomingBuffer[0]);	
		}

	}

	return rt;
}

void parseArgs(int argc, char* argv[])
{
	for(int i = 0; i < argc; i++)
	{
		if(strcmp(argv[i], "-p") == 0)
		{
			portChar = argv[i+1];
			portInt = atoi(argv[i+1]);
			i++;
		}
		//Add additional opts here as above
	}

}

receiveType translateReceiveType(char c)
{
	switch(c)
	{
		case 0:
	fprintf(stdout, " KEYFRAMESEND\n");
			return KEYFRAMESEND;
		break;
		case 1:
	fprintf(stdout, " CLOSECONNECTION\n");
			return CLOSECONNECTION;
		break;
		case 2:
	fprintf(stdout, " EMPTY\n");
			return EMPTY;
		break;
		case 3:
			//Extract length and store in incomingBufferLength
			memcpy(&incomingBufferLength, incomingBuffer+1, 2);
			return NEWKEYSTATE;
		break;
		case 4:
			return KEYBOARDUPDATE;
		break;

	}
	return EMPTY;

}


void sendCharStream(char *stream, int32_t streamSize, char sendType)
{
	fprintf(debuglog, "Sending stream");

	static unsigned char sendID = 255;
	if(sendID == 255)
		sendID = 0;
	else
		sendID++;

	float numOfTransF = (float)streamSize / (float)(TRANSMISSIONSIZE - 2); // include frame order
	unsigned int numberOfTransmissions = (int)ceil(numOfTransF);

	char *streampospoint = stream - STREAMHEADEROFFSET;
	long int streampos = 0;
	int sendlength;
	int packetSent;
	int sleepTimer = 25000 / numberOfTransmissions; //25ms split for each transmission	

	char *datastream = (char*)malloc(sizeof(char)*15);

	char header = (char)HEADER;
	memcpy(datastream, &header, sizeof(char));
	memcpy(datastream+1, &sendType, sizeof(char));
	memcpy(datastream+2, &sendID, sizeof(char));
	memcpy(datastream+3, &streamSize, sizeof(uint32_t));

	sendto(socketDesc, datastream, 15, 0, (sockaddr*)&returnSocketDesc, sizeof(sockaddr_in));
	


	for(short int i = 0; i < numberOfTransmissions; i++)
	{
		memcpy(streampospoint, &sendType, sizeof(char));
		memcpy(streampospoint+1, &sendID, sizeof(char));
		memcpy(streampospoint+2, &i, sizeof(short int));
		sendlength = streamSize - streampos > TRANSMISSIONSIZE - STREAMHEADEROFFSET ? TRANSMISSIONSIZE : streamSize - streampos + STREAMHEADEROFFSET; 
		streampos += sendlength - STREAMHEADEROFFSET;
		packetSent = sendto(socketDesc, streampospoint, sendlength, 0, (sockaddr*)&returnSocketDesc, sizeof(sockaddr_in));
		streampospoint += TRANSMISSIONSIZE - STREAMHEADEROFFSET;
	}

	free(datastream);

	fprintf(debuglog, " Sent %d packets, %ld size sendID: %d\n", numberOfTransmissions, streamSize, sendID);
}



int vectorImageDiff()
{
	vectorCount = 0;
	numOfSamples = (WIDTH / SAMPLESIZEX) * (HEIGHT / SAMPLESIZEY);	
	int numberOfSampleChecks = ((SAMPLECHECKRANGE * 2) * (SAMPLECHECKRANGE * 2));

	fprintf(debuglog, "Sending P Frame!!!\n");

	//int value for each vector move X and Y
	int* sampleDiff = (int*)malloc(sizeof(int) * numberOfSampleChecks);

	int numOfSamples = ((SAMPLECHECKRANGE * 2) * (SAMPLECHECKRANGE * 2));
	for(int i = 0; i < numberOfSampleChecks; i++)
	{
			sampleDiff[i] = 0;
	}
	int diffPos;


	int curPixel = 0;
	for(int i = 0; i < WIDTH * HEIGHT; i++)
	{
		//per pixel
		//Add check to remove bottom/top sample taking
		diffPos = 0;
		for(int x = -SAMPLECHECKRANGE; x < SAMPLECHECKRANGE; x++)
		{
			for(int y = -SAMPLECHECKRANGE; y < SAMPLECHECKRANGE; y++)
			{
				int checkPos = curPixel + (y*3) + (x * HEIGHT * 3);
				//Add additional out of bounds checks for intermediate north/south wraparounds
				if((checkPos < (WIDTH * HEIGHT * 3))&&
					(checkPos >= 0))
				{							
					sampleDiff[diffPos] += abs(curImage[curPixel] - prevImage[checkPos]);
					sampleDiff[diffPos] += abs(curImage[curPixel+1] - prevImage[checkPos+1]);
					sampleDiff[diffPos] += abs(curImage[curPixel+2] - prevImage[checkPos+2]);
				}
				else
				{
					sampleDiff[diffPos] += SAMPLECHECKRANGE;
				}
				diffPos++;
			}
		}

		//assumption that framebuffer reads bottom to top, then left to right

		//check if one full sample completed
		if(i % (SAMPLESIZEX * SAMPLESIZEY) == 0)
		{
			int lowestSample = 0;
			for(int j = 1; j < numberOfSampleChecks; j++)
			{
				if(sampleDiff[j] < sampleDiff[lowestSample])
					lowestSample = j;
			}

			if(sampleDiff[lowestSample] < SAMPLETHRESHOLD)
			{
				//Am I being stupid here with X vs Y division?
				//Add vector
				int vecMoveX = (int)floor(lowestSample / SAMPLESIZEX) - (SAMPLESIZEX / 2);
				int vecMoveY = (int)lowestSample % SAMPLESIZEY - (SAMPLESIZEY / 2);

				int vecPosX = (int)(floor(i / HEIGHT));
				int vecPosY = (int)(i % WIDTH);

				moveVecs[vectorCount]->moveX = vecMoveX;				
				moveVecs[vectorCount]->moveY = vecMoveY;

				moveVecs[vectorCount]->x = vecPosX;
				moveVecs[vectorCount]->y = vecPosY;

				vectorCount++;
			}

			//Add vector, clear vars
			for(int j = 0; j < numberOfSampleChecks; j++)
				sampleDiff[j] = 0;

			//set curPixel to beginning of next sample
			//still to do
			//
			//
			//

		}
		//Advance curPixel 1 row right and sample starting height
		if((i + 1) % SAMPLESIZEY == 0)
				curPixel += (HEIGHT - SAMPLESIZEY) * 3;

		curPixel += 3;
	}
	fprintf(debuglog, "Done\n");
	
	return vectorCount;
}
