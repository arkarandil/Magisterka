#pragma once
#include "stdafx.h"
#include "InputHandler.h"
#include "InputFrame.h"
#include <tchar.h>
#include <stdio.h>

/**
 * @author  Piotr Czerwinski, Maciej Lichon
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Class responsibile for handling camera input. Objects of class use internal InputFrame buffer
 * to cache captured InputFrame 30 ms default capture time
 */

class INPUTHANDLER_API CameraInputHandler:public InputHandler {
public:

	~CameraInputHandler();
	/**
	 * Default constructor. Creates input handler with one camera
	 * buffer size 5, and capture time 30 ms
	 */
	CameraInputHandler():InputHandler(1),_bufferSize(5)
	{
		InitializeInputFrames();
		InitializeCameraInputHandler();
	}
	/**
	 * Constructor creates input handler capture time 30 ms.
	 * @param views number of cameras
	 * @param bufferSize size of internal buffer
	 */
	CameraInputHandler(int views,unsigned int bufferSize):InputHandler(views),_bufferSize(bufferSize)
	{
		InitializeInputFrames();
		InitializeCameraInputHandler();
	}

	void SetCaptureTime(int time){_captureTime=time;}

	/**
	 * Function returns next InputFrame from buffer.
	 * It doesn't have to be InputFrame just captured.
	 * Captured frames are kept in buffer
	 * @return next InputFrame from buffer
	 */
	InputFrame* ProcessNextFrame();

	/**
	 * @param ids of InputFrame which is retrived (openCV camera's id's?)
	 * @return if connecting was successful
	 */
	bool ConnectCaptures(int* ids);

	/**
	 * @param index of InputFrame which is retrived
	 * @return InputFrame of given index
	 */
	virtual InputFrame * ProcessFrameWithIndex(int index)
	{
		//its cycling buffer so index 0 of buffer is not neceserly begining
		int realIndexOffset=GetLastFrameProcessedIndex()-index; //Difference between ca
		int realIndex= _lastCapturesBufferIndex-realIndexOffset;
		if(realIndex<0)
			realIndex=_bufferSize+realIndex;

		return &_InputFrames[realIndex];
	}

	/**
	 * @return index of last InputFrame captured
	 *
	 */
	int GetLatestFrameCapturedIndex()
	{
	if(_lastCapturesBufferIndex>=_beginIndex)
			return (_lastCapturesBufferIndex-_beginIndex);
		else
			return _bufferSize+(_lastCapturesBufferIndex-_beginIndex);
	}
	/**
	 * @return index of last InputFrame processed by ProcessNextFrame.
	 */
	int GetLastFrameProcessedIndex()
	{
		if(_currentIndex>=_beginIndex)
			return (_currentIndex-_beginIndex);
		else
			return _bufferSize+(_currentIndex-_beginIndex);
	}

	/**
	 * Functction creates new thread to handle InputFrame capturing
	 */
	void StartCapturing();

	/**
	 * Stops camera from capturing InputFrames
	 */
	void PauseCapturing()
	{
		ResetEvent(_hPauseEvent);
	}

	/**
	 * Resume capturing InputFrames in camera. Also resets internal indexes in buffer
	 */
	void ResumeCapturing()
	{
		SetEvent(_hPauseEvent);
		_inputPaused=false;
		_beginIndex=0;
		_currentIndex=0;
		_lastCapturesBufferIndex=0;
	}

	/**
	 * Stops video capturing. StartCapturing has to be called to start capturing again
	 */
	void StopCapturing()
	{
		SetEvent(_hPauseEvent);
		_captureVideo=false;
	}

protected:
	friend VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);
	friend DWORD WINAPI TimerThread(PVOID input);

	void InitializeInputFrames();		//initializing buffer,
	void InitializeCameraInputHandler();	//settings pause events

	bool _captureVideo;						// used in InputFrame capturing loop
	bool _inputPaused;						// capturing paused

	InputFrame* _InputFrames;				// internal buffer
	int _bufferSize;						// number of elements in internal buffer
	int _lastCapturesBufferIndex;			// index of last captured InputFrame in buffer  _currentSavedIndex;
	int _beginIndex;						//index of begining of buffer (oldewst frame captured)

	HANDLE _hDoneEvent;						// used in checking if InputFrame was captured
	HANDLE _hPauseEvent;					// used in pausing capturing
	HANDLE	_hTimer;						// timer of InputFrame capturing

	int _captureTime;						// time interval between InputFrames capturing

	bool _cameraInitialized;
};