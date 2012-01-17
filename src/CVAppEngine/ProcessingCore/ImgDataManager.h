#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <map>
#include "InputFrame.h"
#include "Tools.h"
#include <string.h>
#include "boost/filesystem.hpp"
#pragma once

struct PDPC_PROCESSINGCORE_API  ImgSetInfo
{
	int priority;          //0 most important 10 not very iportant
	int _viewsNum;
	int _imgNum;
};

class PDPC_PROCESSINGCORE_API ImgDataManager
 {
  private:
        ImgDataManager() {_workingDir=NULL;}
		ImgDataManager(const ImgDataManager &){}
		ImgDataManager& operator=(const ImgDataManager&){return Instance();}

		std::map<char*,ImgSetInfo*,cmp_str> _sets;
		char* _workingDir;

  public:
		//void CommitNewData(char* name,ImgSet* set);
		void SetWorkingDir(char* name);
		bool CheckDirExist(char* name,int viewsInImg,int imgsNum);
		void AddNewFrame(char* name,InputFrame* frame);

		/*
		Gets Image from current working set, that coresponds
		to given name index on set and view(i.e. stereo has more than one view)
		*/
		IplImage * GetImage(char* name,int index,int view);

		/*
		Gets Image Set Info for given name,
		returns NULL if no info found
		*/
		ImgSetInfo* GetImgSetInfo(char*name);
        static ImgDataManager& Instance()
        {
          static ImgDataManager instance;
          return instance;
        }
 };