struct pixel
{
public:
	char r;
	char b;
	char g;
public:
	pixel (){r = 0; g = 0; b = 0;}
};

struct moveVector
{
public:
	char moveX;
	char moveY;
	char x;
	char y;
	moveVector(){moveX = 0; moveY = 0; x = 0; y = 0;}
	moveVector(char mX, short mY, short cX, short cY){moveX = mX; moveY = mY; x = cX; y = cY;}

};

struct streamHeader
{
public:
	int imageSizeX;
	int imageSizeY;
	char sampleSizeX;
	char sampleSizeY;
	streamHeader(int iX, int iY, int sX, int sY){imageSizeX = iX; imageSizeY = iY; sampleSizeX = sX; sampleSizeY = sY;}
};

//Use to translate incoming requests from client
enum receiveType
{
	KEYFRAMESEND = 0,
	CLOSECONNECTION = 1,
	EMPTY = 2,
};

//Data types to send
enum sendType
{
	HEADER = 0,
	PREDICTIONFRAME = 1,
	KEYFRAME = 2,
};

/*
Rules for each frame send protocol
-: char to indicate send type from enum

//PredictionFrame
-: 4char int numberOfVectors
-: 4chars: samplePosX, samplePosY, moveX, moveY (foreach vec)
-: 4char int, size of delta stream
-: compressed delta stream (lzw or similar)

//Keyframe
-: 4char int size of full image stream
-: full compressed image stream (lzw, possibly low% jpeg or similar)

//Header
-: 4char int imagesizeX
-: 4char int imagesizeY
-: char samplesizex
-: char samplesizey

*/

