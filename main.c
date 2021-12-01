/*
	SketchMDXPlayer	v0.31
	author:ISH
*/
#include    "main.h"
#include "YM2151.h"
#include "MMLParser.h"
#include "MDXParser.h"
#include "compat.h"
#include "main.h"
// IO				io;
struct MDXParser mdx;

int32_t		waittime=0;
uint32_t	proctime=0;

void main()
{
// Serial.begin(9600);
// 	io.Init();
	YM2151_begin();
	MDXParser_Setup(&mdx, 0x2800);
	MDXParser_Elapse(&mdx, 0);

	while (1) {
	    loop();
	}
}

void loop()
{
	waittime = MDXParser_ClockToMicroSec(&mdx, 1);
	waittime -= proctime;
	while(waittime > 0){
		if(waittime > 16383){
			delayMicroseconds(16383);
			waittime -= 16383;
		} else {
			delayMicroseconds(waittime);
			waittime = 0;
		}
	}
	proctime = micros();
	MDXParser_Elapse(&mdx, 1);
	proctime = micros() - proctime;
}
