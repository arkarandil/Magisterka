#pragma once
#include "stdafx.h"
#include "InputHandler.h"
#include "InputFrame.h"
#include <tchar.h>
#include <stdio.h>

/**
 * @author  Piotr Czerwinski
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Class responsibile for handling video input. It is possible to grab data from one or more videos(stereo or more)
 */
class INPUTHANDLER_API VideoInputHandler:public InputHandler
{
public:
	~VideoInputHandler();
	VideoInputHandler():InputHandler(1)
	{
	}
	VideoInputHandler(unsigned int views):InputHandler(views)
	{
		assert(views<100);
		_captureInputFrame=new InputFrame();
		_captureInputFrame->InitializeFrame(_viewsNum);
	}

	//InputHandler implementation
	virtual InputFrame* ProcessNextFrame();
	virtual int GetLatestFrameCapturedIndex(){return _maxIndex;}
	virtual InputFrame * ProcessFrameWithIndex(int index);
	virtual int GetLastFrameProcessedIndex(){return _currentIndex;}

	virtual void StartCapturing(){};
	virtual void PauseCapturing(){};
	virtual void ResumeCapturing(){};
	virtual void StopCapturing(){};

	/**
	 * Loads input from video files
	 * @param array of paths to video files
	 */
	bool ConnectVideos(char* paths[]);
protected:

	InputFrame * _captureInputFrame;
	int _maxIndex;
};