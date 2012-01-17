/*
* author: Maciej Lichoñ
*/
#include "stdafx.h"
#include "CStereoVideo.h"
CStereoVideo::
CStereoVideo()
{
}
/*
*Init the object with config file
*/
CStereoVideo::
CStereoVideo(CAppConfig conf)
{
  _config = conf;
  _leftCamNum=conf._camL;
  _rightCamNum=conf._camR;
}
/*
*Init the object with custom camera numbers
*/
CStereoVideo::
CStereoVideo(int leftCam,int rightCam)
{
  _leftCamNum=leftCam;
  _rightCamNum=rightCam;
}
/*
*Connect to the eyes of the camera specified in the constructor
*/
void CStereoVideo::Connect()
{
	_leftCam = cvCreateCameraCapture (0);//zmieñ w zale¿noœci ile masz kamerek zaczyna siê od 0, pierwsze oko
 	_rightCam = cvCreateCameraCapture (0);//drugie oko
	if(_leftCam == NULL || _rightCam == NULL)
	{//there is no double eye camera, use video files instead
		_leftCam = cvCreateFileCapture(_config._recordIL.c_str());
		_rightCam = cvCreateFileCapture(_config._recordIR.c_str());
		if(_leftCam == NULL || _rightCam == NULL)
		{//no videos defined, we need to use images
			_images  = new StereoInputFrame[_config._InputFrameFiles.size()];
			_InputFrameCounter = 0;
			int i = 0;
			for( list<string>::iterator iter=_config._InputFrameFiles.begin(); iter != _config._InputFrameFiles.end(); ++iter )
			{
				_images[i]._leftIm = cvLoadImage(((string)*iter).c_str(),0);
				iter++;
				_images[i+1]._rightIm = cvLoadImage(((string)*iter).c_str(),0);
				i++;
			}
		}
	}

	//init view windows.
	
}
/*
*Init's a contionious loop that grabs,displays and process data from both eyes of the camera,
*the processing sequence is in the overrited ContiniousProcessInputFrame.
*/
void CStereoVideo::ContiniousCaptureLoop()
{
	StereoInputFrame * f;
for(;;)
	{

		f = GetSingleInputFrame();
  		cvShowImage ("3D Webcam left", f[0]._leftIm);
	
		//cvShowImage ("3D Webcam right", f[0]._rightIm);
	
    

	if(cv::waitKey(30) >= 0) break;//if esc wass pressed break
	}
	cvReleaseCapture (&_leftCam);
	cvReleaseCapture (&_rightCam);
	cvDestroyWindow ("3D Webcam left");
	cvDestroyWindow ("3D Webcam right");
}
/*
*Processing method for each captured InputFrame. not in a new thread
*/
void CStereoVideo::ContiniousProcessInputFrame(StereoInputFrame * InputFrame)
{

}
/*
* grabs a Stereo InputFrame from both eyes. StereoInputFrame is a structure containing InputFrame from right and left eye
*/
StereoInputFrame * CStereoVideo::GetSingleInputFrame()
{
	StereoInputFrame f;
	if(_leftCam != NULL && _rightCam != NULL)
	{//video or device
		int t;
		int y;
 		t=cvGrabFrame(_leftCam);
		
		f._leftIm = cvRetrieveFrame (_leftCam);
		//y=cvGrabInputFrame(_rightCam);
		//f._rightIm = cvRetrieveInputFrame (_rightCam);
	}
	else
	{//given in config files InputFrames
		if(_InputFrameCounter<(_config._InputFrameFiles.size()/2))
		{
			_InputFrameCounter++;
		}
		else
		{
			_InputFrameCounter=0;
		}
		f._leftIm = _images[_InputFrameCounter]._leftIm;
		f._rightIm = _images[_InputFrameCounter]._rightIm;
	
	}
return &f;
}