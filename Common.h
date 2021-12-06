#ifndef YM_COMMON_H_INCLUDED
#define YM_COMMON_H_INCLUDED
#include	"YM2151.h"
#include <stdio.h>

// #define		_DEBUG

#ifdef	_DEBUG
#define		ASSERT(msg)		printf(msg);printf("\n");
#define		PRINTH(msg,data)	printf(msg); (" 0x%04X\n", data);
#else
#define		ASSERT(msg)
#define		PRINTH(msg,data)
#endif

#endif  //YM_COMMON_H_INCLUDED
