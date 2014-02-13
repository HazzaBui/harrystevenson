//Set up colour array, format [] byte byte byte rgb

#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include <string.h>

//#include <png.h>
#include "lodepng.h"
#include "lzfx.h"
#include "lzfP.h"

//Include network stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>


#define WIDTH 640
#define HEIGHT 480
#define SAMPLESIZEX 20
#define SAMPLESIZEY 20

//Signatures
void createVectors(int start, int starty, int sampleSizeX, int sampleSizeY);
void serialiseData();
void flushData();
void setupConnection();
void sendKeyframe();
receiveType checkIncomingStream();
void initialiseVars();
void parseArgs(int argc, char* argv[]);
receiveType translateReceiveType(char c);

//Application vars
unsigned char* prevImage;
unsigned char* curImage;
moveVector** moveVecs;
int numOfSamples;
int vectorCount;
streamHeader* sh;

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
char* sendBuffer;
unsigned int sendBufferLength;
char* sendTypeBuffer;
unsigned int sendTypeBufferLength;
size_t sendSize;


int main(int argc, char* argv[])
{
	fprintf(stdout, "\nStart");
	receiveType rt;
	bool loop = true;
	
	parseArgs(argc, argv);
	initialiseVars();
	setupConnection();

	sendKeyframe();

	fprintf(stdout, "\nBeginning loop ");
fflush(stdout);

	//Use 'loop' to maintain gameloop
	//Perform all compression/communication here
	while(loop)
	{
		rt = checkIncomingStream();

		switch(rt)
		{
			case KEYFRAMESEND:
				sendKeyframe();
				//Send keyframe
			break;
			case CLOSECONNECTION:
				//close down connection and cleanup
				loop = false;
			break;
			default:
				//continue standard, send p/k depending on loop iteration

			break;
		}
	}
	
	//cleanup, run any post process, inform game application etc
}

void initialiseVars()
{
	sh = new streamHeader(WIDTH, HEIGHT, SAMPLESIZEX, SAMPLESIZEY);

	numOfSamples = (SAMPLESIZEX * SAMPLESIZEY) / 100;

	fprintf(stdout, "\nInitial alloc");
	//allocate mem for current and previous image
	prevImage = (unsigned char*)malloc(sizeof(char) * WIDTH * HEIGHT * 3);
	curImage = (unsigned char*)malloc(sizeof(char) * WIDTH * HEIGHT * 3);

	fprintf(stdout, "\nImage size: %d ", WIDTH * HEIGHT * 3);

	//Create in shared memory only need current in shared, previous saved locally
	//Should this be done in image capture?
	//write pointer to file
	//touch sem
	
	//Movement Vector list
	moveVecs = new moveVector*[numOfSamples];
	fprintf(stdout, "\nCreate %d move vectors");

	for(int i = 0; i < numOfSamples; i++)
	{
		moveVecs[i] = new moveVector();
	}
	fprintf(stdout, "\nCreated");

	//Network receive buffer
	incomingBuffer = (char*)malloc(sizeof(char) * 255);

	sendBuffer = (char*)malloc(sizeof(char)* WIDTH * HEIGHT * 3);
	sendBufferLength = WIDTH * HEIGHT * 3;

	sendTypeBuffer = (char*)malloc(sizeof(char)* 15);
	sendTypeBufferLength = 15;

	fprintf(stdout, "\nFinished initialise");
	fflush(stdout);
}

void createVectors(int start, int starty, int sampleSizeX, int sampleSizeY)
{
	vectorCount = 0;
	//For each sample, check nearby image for movement approximation
	for(int i = 0; i < numOfSamples; i++)
	{


		//If new vector, add at position VectorCount++
	}
}





void serialiseData()
{
	//
	//push numOfVectors to stream as 4 byte int
	
	for(int i = 0; i < vectorCount; i++)
	{
		//push each move vector
	}
}

void setupConnection()
{
	fprintf(stdout, "\nUsing ip %d and port %d ", 0, portInt);
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

	socklen_t socklen = (socklen_t)sizeof(sockaddr);

	recvfrom(socketDesc, incomingBuffer, 255, 0, (sockaddr*)&returnSocketDesc, &socklen); 
	
//	fprintf(stdout, "\nClient connected: %s %s ", returnSocketDesc.sin_addr.s_addr, returnSocketDesc.sin_port);
	fprintf(stdout, "\nClient connected ");
}


void sendKeyframe()
{
/*	//loseless compress and send full keyframe
	//no vectors, delta etc
	unsigned error = lodepng_encode32(&sendBuffer, &sendSize, curImage, WIDTH, HEIGHT);

	fprintf(stdout, "\nImage size: %d ", sendSize);

	char* obuf = (char*)malloc(sizeof(char)* WIDTH * HEIGHT * 3);
	unsigned int* olen;
	int error = lzfx_compress(curImage, WIDTH * HEIGHT * 3, obuf, olen); 
	
	fprintf(stdout, "\nError: %d\nsize: %d ", error, &olen);
*/
fprintf(stdout, "\nSendFrame");

	unsigned int length = lzf_compress(curImage, WIDTH * HEIGHT * 3, sendBuffer, sendBufferLength); 

	if(length > 0)
	{
		sendTypeBuffer[0] = (char)KEYFRAME;
		sprintf(sendTypeBuffer + 1, "%d\n", length);
		sendto(socketDesc, sendTypeBuffer, sendTypeBufferLength, 0, (sockaddr*)&returnSocketDesc, sizeof(sockaddr_in));
		sendto(socketDesc, sendBuffer, sendBufferLength, 0, (sockaddr*)&returnSocketDesc, sizeof(sockaddr_in));
		fprintf(stdout, "\nSent data stream ");
	}

	fprintf(stdout, "\nError: %d\nsize: %d ", length, sendBufferLength);
}

//Check the stream for connection close, resend etc
receiveType checkIncomingStream()
{
	receiveType rt = EMPTY;

	if(poll(&pfd, 1, 1) != 0)
	{
		int size = 	recv(socketDesc, incomingBuffer, 255, 0);
		fprintf(stdout, "\n\nData received: %d \n%s end\n\n", size, incomingBuffer);
		fflush(stdout);

//		char* msg = (char*)malloc(sizeof(char) * 7);
//		sprintf(msg, "Return message");	
//		int len = strlen(msg);
//		sendto(socketDesc, msg, len, 0, returnSocketDesc, sizeof(SOCKADDR_IN));
		//sendto(socketDesc, msg, len, 0, (sockaddr*)&returnSocketDesc, sizeof(sockaddr_in));
//fprintf(stdout, "\nSend msg: %s ", msg);
//fflush(stdout);
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
	fprintf(stdout, "\nTranslating: %d ", c);
	switch(c)
	{
		case '0':
			return KEYFRAMESEND;
		break;
		case '1':
			return CLOSECONNECTION;
		break;
		case '2':
			return EMPTY;
		break;
	}
return EMPTY;

}
