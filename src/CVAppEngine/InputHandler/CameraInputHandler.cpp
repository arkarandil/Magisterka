#include "stdafx.h"
#include "CameraInputHandler.h"
#include <process.h>
#include "Tools.h"
using namespace std;

void CameraInputHandler::InitializeCameraInputHandler()
{
	_beginIndex=0;
	_lastCapturesBufferIndex=0;
	_captureVideo=true;
	_hPauseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	_captureTime=30;
	_cameraInitialized=false;
	_inputPaused=false;
}

void CameraInputHandler::InitializeInputFrames()
{
	assert(_bufferSize>0);
	assert(_viewsNum>0);
	_InputFrames=new InputFrame[_bufferSize];

	for(int i=0;i<_bufferSize;i++)	//initialize all frames in buffer
		_InputFrames[i].InitializeFrame(_viewsNum);
}

bool CameraInputHandler::ConnectCaptures(int* IDs){
	assert(_bufferSize>0);
	assert(_InputFrames!=NULL);
	for(int i=0;i<_bufferSize;i++){	//connect cameras to all frames in buffer
		if(!_InputFrames[i].ConnectCameraCaptures(IDs))
			return false;
	}
	_cameraInitialized=true;
	return true;
}

DWORD WINAPI TimerThread(PVOID i){
	DWORD dw;
	CameraInputHandler* input=(CameraInputHandler*)i;

	/* configuring timer. Interval taken from inputHandler*/
	input->_hTimer=CreateWaitableTimer(NULL,FALSE,NULL);
	LARGE_INTEGER li;
	li.QuadPart=-(input->_captureTime*1000);
	StopWatch watch;
	__int64 time;
	SetWaitableTimer(input->_hTimer,&li,input->_captureTime,NULL,NULL,FALSE);

	while(input->_captureVideo){
		dw=WaitForSingleObject(input->_hTimer, input->_captureTime*2);
		if(dw==WAIT_TIMEOUT)
			cout<<"Wait timeout"<<endl;

		/* Moving beging index forward if buffer is too small */
		if((input->_lastCapturesBufferIndex+1)%input->_bufferSize==input->_beginIndex)
			input->_beginIndex=(input->_beginIndex+1)%input->_bufferSize;

		/* Capturing frames */
		InputFrame* InputFrame= &input->_InputFrames[input->_lastCapturesBufferIndex];
		assert(InputFrame != NULL);

		for(int i=0;i<InputFrame->_viewsNum;i++)
			cvGrabFrame(InputFrame->_captures[i]);

		for(int i=0;i<InputFrame->_viewsNum;i++)
			InputFrame->_views[i]= cvRetrieveFrame (InputFrame->_captures[i]);

		/* Moving index of last capturend in buffer forward */
		input->_lastCapturesBufferIndex=(input->_lastCapturesBufferIndex+1)%input->_bufferSize;

		/* For GetNextIputFrame()*/
		PulseEvent(input->_hDoneEvent);

		/* Processing input is to slow, increasing capture time */
		if(input->_currentIndex==(input->_lastCapturesBufferIndex+1)%input->_bufferSize)
		{
#ifdef _DEBUG
			cout<<"Processing input is to slow, increasing capture time"<<input->_captureTime<<endl;
#endif
			input->_captureTime+=1;
			LARGE_INTEGER li;
			li.QuadPart=-(input->_captureTime*1000);
			SetWaitableTimer(input->_hTimer,&li,input->_captureTime,NULL,NULL,FALSE);
			input->_currentIndex=(input->_currentIndex+1)%input->_bufferSize;
		}
		/* Pausing capturing*/
		WaitForSingleObject(input->_hPauseEvent, INFINITE);
	}
	return 0;
}

/**
 * Creates capturing thread
 */
void CameraInputHandler::StartCapturing(){
	if(!_cameraInitialized)
		return;
	_hDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	chBEGINTHREADEX(NULL,0,&TimerThread,this,0,NULL); //starting thread
}

InputFrame *CameraInputHandler::ProcessNextFrame(){
	if(!_cameraInitialized)
		return NULL;
	DWORD res=WAIT_OBJECT_0;
	InputFrame*InputFrame;

	if(_currentIndex==_lastCapturesBufferIndex){ //no new frame in buffer, wait for camera to capture
		res=WAIT_TIMEOUT;
		if(!_inputPaused)	//no point waiting if paused
			res=WaitForSingleObject(_hDoneEvent, _captureTime*10);
	}
	switch(res){
		case WAIT_OBJECT_0:	// frame was captured
			InputFrame=&_InputFrames[_currentIndex];
			this->_currentIndex=(this->_currentIndex+1)%this->_bufferSize; //processed frame index
			break;
		default:
			_inputPaused=true;
			InputFrame=NULL;
			break;
	}

	return InputFrame;
}
CameraInputHandler::~CameraInputHandler()
{
	delete [] _InputFrames; //deleting buffer
}