// InputHandler.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "InputHandler.h"

INPUTHANDLER_API int nInputHandler=0;

InputHandler::InputHandler()
{
	_currentIndex=0;
	return;
}
unsigned int InputHandler::GetViewsNum(){return _viewsNum;}