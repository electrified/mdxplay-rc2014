/*
	SketchMDXPlayer	v0.31
	author:ISH
*/
#include "main.h"
#include "YM2151.h"
#include "MMLParser.h"
#include "MDXParser.h"
#include "compat.h"
#include "main.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct MDXParser mdx;

int32_t waittime = 0;
uint32_t proctime = 0;
char *buffer;

int main(int argc, char **argv)
{
    // char *buffer;
    printf("MDXPLAY for RC2014\n");

    for (int i = 0; i < argc; ++i)
    {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    YM2151_begin();

    // load file
    readfileintoram(argv[1], &buffer);


    printf("BUffer 2 %d\n", buffer);

    MDXParser_Setup(&mdx, 0);
    MDXParser_Elapse(&mdx, 0);

    while (1)
    {
        loop();
    }
    return EXIT_SUCCESS;
}

void loop()
{
    waittime = MDXParser_ClockToMilliSec(&mdx, 1);
    //   waittime -= proctime;
    while (waittime > 0)
    {
        if (waittime > 16)
        {
            // __SCCZ80
            #ifdef __SDCC
            msleep(16);
            #endif

            waittime -= 16;
        }
        else
        {
            #ifdef __SDCC
            msleep(waittime);
            #endif
            waittime = 0;
        }
    }
    //   proctime = micros();
    MDXParser_Elapse(&mdx, 1);
    //   proctime = micros() - proctime;
}

void readfileintoram(char *filename, char **buffer)
{
    printf("Beginning file load\n");
    FILE *pFile;
    long lSize;
    size_t result;

    pFile = fopen(filename, "rb");
    if (pFile == NULL)
    {
        fputs("File error\n", stderr);
        exit(1);
    }

    // obtain file size
    fseek(pFile, 0, SEEK_END); // seek to the end of the file
    lSize = ftell(pFile);      // get the current file pointer
    rewind(pFile);             // rewind to the beginning of the file

    printf("File size %d\n", lSize);
    // allocate memory to contain the whole file:
    // add one more byte for the NULL character to
    // terminate the memory string

    char *newBuffer = (char *)malloc(sizeof(char) * lSize + 1);
    if (newBuffer == NULL)
    {
        fputs("Memory error", stderr);
        return;
        // exit(2); // different error code from the earlier
        // failed to open the file - why?
    }

    // set the memory to zero before you copy in.
    // the lSize + 1 byte will be 0 which is NULL '\0'
    // note, we clear memory and add the NULL at the same time

    memset(newBuffer, 0, sizeof(char) * lSize + 1);

    // copy the file into the buffer:
    result = fread(newBuffer, sizeof(char), lSize, pFile);
    printf("result %d\n", result);
    if (result != lSize)
    {
        fputs("Reading error", stderr);
        return;
        // exit(3); // we use different exit codes for different errrors, that's why.
    }

    // the whole file is now loaded in the memory buffer. Is it correct?
    //
    // look at the file using the cat myfile.txt. Problem is you
    // can't see hidden whitespace characters like newline or NULL.
    // try, od -t c myfile.txt. Now you can see the complete
    // file. What is at the end of the file? Probably a \n (newline).

    // Why not run gdb and look at the buffer contents.
    // Looks good. OK.
    // Let's print it out

    // fputs(buffer, stdout);

    // terminate

    fclose(pFile);
    // free(buffer);
    printf("BUffer %d\n", newBuffer);
    printf("Loading file finished\n");
    *buffer = newBuffer;
}
