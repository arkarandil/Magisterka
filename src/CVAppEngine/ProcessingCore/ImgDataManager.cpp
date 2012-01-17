#include "stdafx.h"
#include "ImgDataManager.h"
using namespace std;
using namespace boost::filesystem;

void ImgDataManager::SetWorkingDir(char* newWorkingDir)
{
	assert(newWorkingDir!=NULL);
	_workingDir = newWorkingDir;
}

void ImgDataManager::AddNewFrame(char* name,InputFrame* frame)
{
	assert(_workingDir!=NULL);
	ImgSetInfo* set;
	map<char*,ImgSetInfo*,cmp_str>::iterator it;
	it=_sets.find(name);
	int frameIndex=0;

	if(it==_sets.end())
	{
		set=new ImgSetInfo();
		set->_imgNum=1;
		set->_viewsNum=frame->_viewsNum;
		set->priority=0;
		_sets.insert(map<char*,ImgSetInfo*>::value_type(name,set));
		if ( exists( current_path()/_workingDir/name) )
			remove_all( current_path()/_workingDir/name );
		if ( !exists( current_path()/_workingDir) )
			create_directory(current_path()/_workingDir);
		create_directory(current_path()/_workingDir/name);
	}
	else{
		set=it->second;
		frameIndex=(set->_imgNum)++;
	}
	char buffer[MAX_PATH];
	for(int i=0;i<frame->_viewsNum;i++){
		sprintf(buffer,"%s/%s/%d.%d.png",_workingDir, name,frameIndex,i);
		cvSaveImage(buffer ,frame->_views[i]);
		//cvReleaseImage(&_frame[i]);
	}
}

bool ImgDataManager::CheckDirExist(char* name,int viewsInImg,int imgsNum)
{
	if ( exists( current_path()/_workingDir/name) )
	{
		map<char*,ImgSetInfo*,cmp_str>::iterator it;
		it=_sets.find(name);
		if(it!=_sets.end())
			return true;
		ImgSetInfo *set=new ImgSetInfo();
		set->_imgNum=imgsNum;
		set->_viewsNum=viewsInImg;
		set->priority=0;
		_sets.insert(map<char*,ImgSetInfo*>::value_type(name,set));
		return true;
	}
	return false;
}

IplImage * ImgDataManager::GetImage(char* name,int index,int view)
{
	assert(_workingDir!=NULL);

	map<char*,ImgSetInfo*,cmp_str>::iterator it;
	it=_sets.find(name);
	if(it==_sets.end())
		return NULL;
	if(index<0||index>=it->second->_imgNum)
		return NULL;
	char buffer[MAX_PATH];
	sprintf(buffer,"%s/%s/%d.%d.png",_workingDir,name,index,view);
	IplImage* img=cvLoadImage(buffer);
	return img;
}

ImgSetInfo* ImgDataManager::GetImgSetInfo(char*name)
{
	map<char*,ImgSetInfo*,cmp_str>::iterator it;
	it=_sets.find(name);
	if(it==_sets.end())
		return NULL;
	return it->second;
}