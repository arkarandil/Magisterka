#include "stdafx.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include "ITask.h"
#include "InputFrame.h"
#include "VideoInputHandler.h"
#include "ImgDataManager.h"
#include <string>
#pragma once

class CaptureVideoTask:public ITask
{
public:
	CaptureVideoTask(int views,char** paths,int start=0,int count=MAXINT):
	  m_views(views),m_paths(paths),m_startIndex(start),m_framesCount(count){}
	void Perform(){
		VideoInputHandler *video =new VideoInputHandler(m_views);
		if(!video->ConnectVideos(m_paths))
		{
			std::cout<<"Wrong video path"<<std::endl;
			system("PAUSE");
			return ;
		}
		if(ImgDataManager::Instance().CheckDirExist("original",m_views,m_framesCount))
			return;
		InputFrame * f;
		f = video->ProcessFrameWithIndex(m_startIndex);
		IplImage ** originalFrameCopy=new IplImage * [m_views];
		while(f!=NULL && m_framesCount-->0){
			for(int j=0;j<m_views;j++){
				if(f->_views[j]==NULL)
					goto allLoaded;
				originalFrameCopy[j] = cvCreateImage( cvSize( f->_views[j]->width, f->_views[j]->height ),
					f->_views[j]->depth, f->_views[j]->nChannels );
				cvCopy(f->_views[j], originalFrameCopy[j], NULL );
			}
			ImgDataManager::Instance().AddNewFrame("original",f);
			f = video->ProcessNextFrame();
		}
allLoaded:
		delete[]originalFrameCopy;
	}
private:
	int m_views;
	int m_startIndex;
	int m_framesCount;
	char** m_paths;
};

class ConnectVideoTask:public ITask{
public:
	ConnectVideoTask(int views,char** paths):
	  m_views(views),m_paths(paths){}
	void Perform(){
	}
private:
	int m_views;
	char** m_paths;
};

class DialogBoxTask:public ITask{
public:
	DialogBoxTask(int i):m_time(i){}
	void Perform(){
		char buff[256];
		_itoa(m_time,buff,10);
		Sleep(m_time*1000);
		MessageBoxA(NULL,buff, NULL, NULL);
	}
private:
	int m_time;
};