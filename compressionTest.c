

#include "lzfP.h"
#include <stdint.h>
#include <stdio.h>
#include <jpeglib.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

//functions
void calcJpeg(char* file, char* dummy);
void calcLZW(char* stream, char* dummydata);

int main()
{

	char* buffer = (char*)malloc(sizeof(char) * 640 * 480 * 3);
	char* dummydata = (char*)malloc(sizeof(char) * 640 * 480 * 6);
	
	int numberOfLoops = 1000;
	FILE* fp = fopen("framebuffer", "r");

	fread(buffer, sizeof(char), 640*480*3, fp);
	
	fprintf(stdout, "\nPerforming %d iterations of JPEG and LZW compression\n", numberOfLoops);

	struct timespec looptimer;
	long timeelapsed = 0;
	long timeinms = 0;



	for(int i = 0; i < numberOfLoops; i++)
	{
		clock_gettime(CLOCK_MONOTONIC, &looptimer);
		timeelapsed = looptimer.tv_nsec;


		calcJpeg(buffer, dummydata);



		clock_gettime(CLOCK_MONOTONIC, &looptimer);
		timeelapsed = looptimer.tv_nsec - timeelapsed;
		timeelapsed = timeelapsed < 0 ? timeelapsed += 1000000000 : timeelapsed;
//		timeinms += timeelapsed;
		timeinms += timeelapsed / 1000000;
	}

	timeinms /= numberOfLoops;
	fprintf(stdout, "\nJPEG: Average compression time:%ldms:\n", timeinms);

	timeelapsed = 0;
	timeinms = 0;


	for(int i = 0; i < numberOfLoops; i++)
	{
		clock_gettime(CLOCK_MONOTONIC, &looptimer);
		timeelapsed = looptimer.tv_nsec;



		calcLZW(buffer, dummydata);



		clock_gettime(CLOCK_MONOTONIC, &looptimer);
		timeelapsed = looptimer.tv_nsec - timeelapsed;
		timeelapsed = timeelapsed < 0 ? timeelapsed += 1000000000 : timeelapsed;
		timeinms += timeelapsed / 1000000;
	//	timeinms += timeelapsed;
	}
	timeinms /= numberOfLoops;

	fprintf(stdout, "LZW: Average compression time:%ldms:\n", timeinms);
}

void calcJpeg(char* file, char* dummy)
{

	FILE* outfile = fopen("/tmp/test.jpeg", "wb");

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = 640;
	cinfo.image_height     = 480;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	/*set the quality [0..100]  */
	jpeg_set_quality (&cinfo, 75, true);
	jpeg_start_compress(&cinfo, true);



	JSAMPROW row_pointer;          /* pointer to a single row */
 
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer = (JSAMPROW) &file[cinfo.next_scanline*(3>>3)*640];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);



}



void calcLZW(char* stream, char* dummy)
{
	unsigned int length = lzf_compress(stream, 640*480*3, dummy, 640*480*6); 
}
