#pragma once
#include "stdafx.h"
#include <opencv2\opencv.hpp>

/**
 * @author  Piotr Czerwinski
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Structure represents InputFrame of image captured from source. InputFrame is able to
 * store input from one or more input sources captured at the same time
 */
struct INPUTHANDLER_API InputFrame {
public:

	void InitializeFrame(unsigned int viewsNum)
	{
		_viewsNum=viewsNum;
		_views=new IplImage* [_viewsNum];
	}

	bool ConnectCameraCaptures(int * captureIDs)
	{
		assert(_viewsNum>0);
		_captures=new CvCapture*[_viewsNum];
		for(int j=0;j<_viewsNum;j++)
		{
			_captures[j] = cvCreateCameraCapture (captureIDs[j]);
			if(_captures[j]==NULL)// connecting failed
				return false;
		}
		return true;
	}
	bool ConnectVideoCaptures(char* paths[])
	{
		assert(_viewsNum>0);
		_captures=new CvCapture*[_viewsNum];
		for(unsigned int j=0;j<_viewsNum;j++)
		{
				_captures[j] = cvCreateFileCapture (paths[j]);
				if(_captures[j]==NULL)
					return false;
		}
		return true;
	}

	~InputFrame()
	{
		assert(_viewsNum>0);
		for(int j=0;j<_viewsNum;j++)
		{
			cvReleaseCapture(&_captures[j]);
			_captures[j] =NULL;
			delete _views[j];
		}
		delete[] _views;
		delete[] _captures;
	}
	/**captured views
	*/
	IplImage **_views;
	/**
	 * sources used to capture InputFrame
	 */
	CvCapture **_captures;
	/**
	 * number of input sources
	 */
	unsigned int _viewsNum;
};