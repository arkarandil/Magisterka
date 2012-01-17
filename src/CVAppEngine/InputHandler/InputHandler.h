#pragma once
#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include "InputFrame.h"

/**
 * @author  Piotr Czerwinski
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Interface used to handle image input
 */

class INPUTHANDLER_API InputHandler {
public:
	InputHandler(void);
	InputHandler(unsigned int views):_viewsNum(views){_currentIndex=0;}
	virtual ~InputHandler(){};

	/**
	 * @return index of InputFrame captured from source //old getViewewsNum
	 */
	unsigned int GetViewsNum();

	/**
	 * Function returns InputFrame of given index
	 * @return next InputFrame
	 */
	virtual InputFrame * ProcessFrameWithIndex(int index)=0;

	/**
	 * Function gets and process next InputFrame from input source
	 * Should be called every frame //Next input frame
	 * @return next InputFrame
	 */
	virtual InputFrame* ProcessNextFrame()=0;

	/**
	 * @return index of most recent InputFrame captured from source
	 */
	virtual int GetLatestFrameCapturedIndex()=0;
	/**
	 * @return index of most recent  InputFrame processed by ProcessNextFrame
	 */
	virtual int GetLastFrameProcessedIndex()=0;

	virtual void StartCapturing()=0;
	virtual void PauseCapturing()=0;
	virtual void ResumeCapturing()=0;
	virtual void StopCapturing()=0;

protected:
	int _currentIndex;		//index of recent captured file
	unsigned int _viewsNum;	//number of camera views (stereo)
};