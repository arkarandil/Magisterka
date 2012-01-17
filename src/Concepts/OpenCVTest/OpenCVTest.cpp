// OpenCVTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include "InputHandler.h"
#include "InputFrame.h"
#include "CameraInputHandler.h"
#include "VideoInputHandler.h"
using namespace std;
using namespace cv;
#include <direct.h>

int _tmain(int argc, _TCHAR* argv[])
{
	/*cout<<"Loading Config "<< endl;
	CAppConfig * conf = new CAppConfig("conf.cfg");
	cout<< "Testing OpenCV CStereoVideo class"<<endl;
	CStereoVideo * c = new CStereoVideo(0,1);
	c[0].Connect();
	c[0].ContiniousCaptureLoop();
	cout<< "done. by Maciej Lichon and Piotr Czerwinski"<<endl;
	system("PAUSE");*/
	int IDs[]={0};
	char* paths[]={"C:\\t.flv"};
	/*CameraInputHandler* camera=new CameraInputHandler(1,3);
	if(!camera->ConnectCameras(IDs)){
		cout<<"Wrong camera IDs"<<endl;
		system("PAUSE");
		return -1;
	}*/
	VideoInputHandler*camera =new VideoInputHandler(1);
	if(!camera->ConnectVideos(paths)){
		cout<<"Wrong camera IDs"<<endl;
		system("PAUSE");
		return -1;
	}
	
	InputHandler *input=camera;
	camera->StartCapturing();
	cvNamedWindow ("3D Webcam left", CV_WINDOW_AUTOSIZE);
	//cvNamedWindow ("3D Webcam right", CV_WINDOW_AUTOSIZE);
	InputFrame * f;

  /* generate secret number: */
  int lastInputFrame=0;
int key;
	while(true){

		key=cv::waitKey(5);
		switch(key){
			case 'q':
				goto finish;
			case 'p':
				input->PauseCapturing();
				break;
			case 'r':
				input->ResumeCapturing();
				break;
			case 'b':
				if(lastInputFrame>0)
					lastInputFrame--;
				break;
			case 'f':
				if(lastInputFrame<input->GetLastInputFrameIndex()-1)
					lastInputFrame++;
				break;
		}
		f = input->GetNextInputFrame();
		if(f!=NULL){
			cvShowImage ("3D Webcam left", f->m_views[0]);
			//cvShowImage ("3D Webcam right", f->m_views[1]);
			lastInputFrame=MAX(0,input->GetLastInputFrameIndex()-1);
		}
		else{
			f=input->GetInputFrameByIndex(lastInputFrame);
			if(f!=NULL){
				cvShowImage ("3D Webcam left", f->m_views[0]);
				//cvShowImage ("3D Webcam right", f->m_views[1]);
			}
		}
		
	}
finish:
	input->StopCapturing();
	system("PAUSE");
	delete camera;
	return 0;
}

