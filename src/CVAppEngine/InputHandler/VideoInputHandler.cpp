#include "stdafx.h"
#include "VideoInputHandler.h"
using namespace std;

VideoInputHandler::~VideoInputHandler()
{
	delete _captureInputFrame;
}

//InputHandler implementation
InputFrame * VideoInputHandler::ProcessFrameWithIndex(int index)
{
	assert(_captureInputFrame!=NULL);
	index=MAX(0,index);
	index=MIN(_maxIndex-1,index);
	int diff = 0;
	for(unsigned int j=0;j<_viewsNum;j++)
	{
		//if(index!=m_currentIndex)
		assert(_captureInputFrame->_captures[j] !=NULL);
		cvSetCaptureProperty(_captureInputFrame->_captures[j],CV_CAP_PROP_POS_FRAMES,index);

		_captureInputFrame->_views[j]= cvQueryFrame( _captureInputFrame->_captures[j] );
		int realIndex=(int)cvGetCaptureProperty(_captureInputFrame->_captures[j],CV_CAP_PROP_POS_FRAMES);
		diff = index - realIndex;
		if(diff>0) //pobrali�my klatk� kluczow� przed szukana klatk�
		{
			while(diff>0)
			{
				_captureInputFrame->_views[j]= cvQueryFrame( _captureInputFrame->_captures[j] );
				diff--;
			}
		}
		else if(diff<0) //klatka po szukanej klatce, rozpoczynamy cofanie a� znajdziemy klatk� kluczow� przed
		{
			int goBackward = 0;
			while(diff<0 && goBackward<32) //maksymalnie o 32 do ty�u lecimy
			{
				goBackward++;
				cvSetCaptureProperty(_captureInputFrame->_captures[j],CV_CAP_PROP_POS_FRAMES,index-goBackward);
				_captureInputFrame->_views[j]= cvQueryFrame( _captureInputFrame->_captures[j] );
				realIndex=(int)cvGetCaptureProperty(_captureInputFrame->_captures[j],CV_CAP_PROP_POS_FRAMES);
				diff = index - realIndex;
			}
			while(diff>0)
			{
				_captureInputFrame->_views[j]= cvQueryFrame( _captureInputFrame->_captures[j] );
				diff--;
			}
		}
	}
	_currentIndex=index+1;
	return _captureInputFrame;
}

InputFrame * VideoInputHandler::ProcessNextFrame()
{
	if(_currentIndex>=_maxIndex-1)
		return NULL;
	for(unsigned int j=0;j<_viewsNum;j++)
	{
		_captureInputFrame->_views[j]= cvQueryFrame( _captureInputFrame->_captures[j] );
	}
	_currentIndex=(int)cvGetCaptureProperty(_captureInputFrame->_captures[0],CV_CAP_PROP_POS_FRAMES);
	//m_currentIndex++;
	return _captureInputFrame;
}

bool VideoInputHandler::ConnectVideos(char* paths[])
{
	if(!_captureInputFrame->ConnectVideoCaptures(paths))
		return false;

	//search for shortest video
	int temp;
	int minLength=MAXINT;
	for(unsigned int j=0;j<_viewsNum;j++)
	{
		temp = (int) cvGetCaptureProperty(_captureInputFrame->_captures[j],CV_CAP_PROP_FRAME_COUNT);
		if(minLength>temp)
			minLength=temp;
	}
	_maxIndex=minLength;
	return true;
}