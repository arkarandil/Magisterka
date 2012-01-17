// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
// 476 lines

#pragma once
#ifdef INPUTHANDLER_DLL_EXPORT
#define INPUTHANDLER_API __declspec(dllexport)
#else
#define INPUTHANDLER_API __declspec(dllimport)
#endif
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#define MAX_PATH 256

// TODO: reference additional headers your program requires here