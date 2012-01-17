// OpenCVTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include "InputHandler.h"
#include "InputFrame.h"
#include "CameraInputHandler.h"
#include "VideoInputHandler.h"
#include "ITaskScheduler.h"
#include "FIFOTaskQueue.h"
#include "ITask.h"
#include "ImgDataManager.h"
#include <direct.h>

void TaskShedulerTest();

using namespace std;
using namespace cv;

int _tmain(int argc, _TCHAR* argv[])
{
	TaskShedulerTest();
	ImgDataManager::Instance().SetWorkingDir("temporary");

	int IDs[]={0};
	char* paths[]={"H:\\Dyplom\\Movies\\MVI_0195.MOV"};
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

	int lastInputFrame=0;
	int key;
	bool paused = false;
	bool manualControl = false;
	while(true)
	{
		key=cv::waitKey(5);
		switch(key)
		{
			case 'q':
				goto finish;
			case 'p':
				input->PauseCapturing();
				paused=true;
				break;
			case 'r':
				input->ResumeCapturing();
				paused=false;
				break;
			case 'b':
				if(lastInputFrame>0)
				{
					lastInputFrame--;
					manualControl = true;
				}
				break;
			case 'f':
				if(lastInputFrame<input->GetLatestFrameCapturedIndex()-1)
				{
					lastInputFrame++;
					manualControl = true;
				}
				break;
		}
		if(!paused && !manualControl)
		{
			f = input->ProcessNextFrame();
			if(f!=NULL)
			{
				ImgDataManager::Instance().AddNewFrame("test",f);
				IplImage* im = ImgDataManager::Instance().GetImage("test",lastInputFrame,0);
				cvShowImage ("3D Webcam left", im);
				//cvShowImage ("3D Webcam right", f->m_views[1]);
				lastInputFrame=MAX(0,input->GetLastFrameProcessedIndex()-1);
			}
		}
		else if(manualControl)
		{
			f=input->ProcessFrameWithIndex(lastInputFrame);
			if(f!=NULL)
			{
				cvShowImage ("3D Webcam left", f->_views[0]);
				//cvShowImage ("3D Webcam right", f->m_views[1]);
			}
			manualControl = false;
		}
	}
finish:
	input->StopCapturing();
	delete camera;
	return 0;
}

int *a,*b;
class IncrementTask:public ITask
{
public:
	IncrementTask(int* i):_value(i){}
	void Perform()
	{
		cout<<"Increment begun"<<endl;
		Sleep(100);
		(*_value)++;
		cout<<"Increment finished"<<endl;
	}
private:
	int* _value;
};
void TaskShedulerTest()
{
	a=new int();
	*a=0;
	b=new int();
	*b=0;
	ITaskScheduler* queue = new FIFOTaskQueue();
	ITask* task1 = new IncrementTask(a);
	ITask* task2 = new IncrementTask(b);
	ITask* task3 = new IncrementTask(a);

	assert(*a==0);
	assert(*b==0);
	queue->ScheduleNewTask(&task1);
	queue->ScheduleNewTask(&task2);
	queue->ScheduleNewTask(&task3);
	assert(*a==0);
	assert(*b==0);

	queue->StartWork();
	assert(*a==0);
	assert(*b==0);

	//zak³adam ¿e task trwa 0.1 sec
	Sleep(110);
	assert(*a==1);
	assert(*b==0);
	Sleep(110);
	assert(*a==1);
	assert(*b==1);
	Sleep(110);
	assert(*a==2);
	assert(*b==1);
	queue->StopWork();
}
/*
Increment after one second
*/