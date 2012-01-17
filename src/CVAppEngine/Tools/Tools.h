// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the TOOLS_CameraPathGraphic
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// TOOLS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

// Tools.h

#include <process.h>
#include "Stdafx.h"
#include <string.h>

#pragma once

struct TOOLS_API cmp_str
{
   bool operator()(char const *a, char const *b)
   {
	  return strcmp(a, b) < 0;
   }
};

/**
 * @author  Piotr Czerwinski
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Simple watch for time measurment in mili and mikroseconds
 */
class TOOLS_API StopWatch{
public:
	StopWatch(){QueryPerformanceFrequency(&m_PerfFreq);Start();}
	void Start(){QueryPerformanceCounter(&m_PerfStart);}

	__int64 Now()const{
		LARGE_INTEGER perfNow;
		QueryPerformanceCounter(&perfNow);
		return(((perfNow.QuadPart - m_PerfStart.QuadPart)*1000)
			/m_PerfFreq.QuadPart);
	}

	__int64 NowInMicro()const{
		LARGE_INTEGER perfNow;
		QueryPerformanceCounter(&perfNow);
		return(((perfNow.QuadPart - m_PerfStart.QuadPart)*1000000)
			/m_PerfFreq.QuadPart);
	}

private:
	LARGE_INTEGER m_PerfFreq;
	LARGE_INTEGER m_PerfStart;
};

/* type used during thread creating */
typedef unsigned (__stdcall *PTHREAD_START)(void*);
#define chBEGINTHREADEX(psa,cbStackSize, pfnStartAddr,	\
	pvParam,dwCreateFlags,pdwThreadID)					\
	((HANDLE)_beginthreadex(						\
	(void*)(psa),									\
	(unsigned)(cbStackSize),						\
	(PTHREAD_START)pfnStartAddr,					\
	(void*)(pvParam),								\
	(unsigned) (dwCreateFlags),						\
	(unsigned*)(pdwThreadID)))