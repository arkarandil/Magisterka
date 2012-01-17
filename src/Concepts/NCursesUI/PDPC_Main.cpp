#include "stdafx.h"
#include <iostream>
#include <tchar.h>
#include <Shlwapi.h>
#include "PDPC_Interface.h"
#include "ITask.h"
#include "PDPC_JobsSet.h"
using std::cout;
using std::endl;

void printHelp(){
	cout<<"Pomoc!"<<endl;
	system("Pause");
}

/**
* Get camera IDs from string array. Cameras IDs are integer numbers.
* Parsing stops at first non integer value
* @param IDs NULL should be passed, initialized during function execution.
*			if no proper id found is set to NULL
* @param argv string array to be parsed
* @param index in argv from which begin parsing
* @param maxIndex in argv
* @return number of proper camera ids found, also size of IDs array
*/
int parseCameraIDs(int ** IDs,_TCHAR* argv[],int index,int maxIndex);
int _tmain(int argc, _TCHAR* argv[])
{
	/*if(argc==1){
		printHelp();
		return -1;
	}*/

	int currArg=1;
	_TCHAR* szDstFileName=NULL;
	_TCHAR**szSrcVideo=NULL;
	PSTR* cSrcVideos;
	int inputFilesNum=0;
	int *cameraIDs=NULL;
	int InputFrameNum=MAXINT;
	int viewsNum=0;
	int startIndex=0;
	bool bAnalyzeVideo=false;
	bool bAnalyzeCamera=false;
	while(currArg<argc){
		if(argv[currArg][0]!=TEXT('-')||_tcslen(argv[currArg])!=2){
			printHelp();
			return -1;
		}

		switch(argv[currArg][1]){
			case TEXT('c'):{	//camera
				if(bAnalyzeVideo){
					printHelp();
					return -1;
				}
				bAnalyzeCamera=true;
				currArg+=parseCameraIDs(&cameraIDs,argv,currArg+1,argc);
				if(cameraIDs==NULL){
					cameraIDs=new int[1];
					cameraIDs[0]=0;
				}

				break;
			}
			case TEXT('o'):{		//Output file
				break;
			}
			case TEXT('f'):{		//InputFrame number
				if(currArg+1<argc&&iswdigit(argv[currArg+1][0])){
					currArg++;
					InputFrameNum=_wtoi(argv[currArg]);
				}
				break;
			}
			case TEXT('s'):{		//Start
				if(currArg+1<argc&&iswdigit(argv[currArg+1][0])){
					currArg++;
					startIndex=_wtoi(argv[currArg]);
				}
				break;
			}
			case TEXT('v'):{		//Video
				if(bAnalyzeCamera){
					printHelp();
					return -1;
				}
				currArg++;
				szSrcVideo=&argv[currArg];
				while(currArg<argc && argv[currArg][0]!=TEXT('-')){
					currArg++;
					viewsNum++;
				}
				currArg--;
				bAnalyzeVideo=true;
				break;
			}
			default:{
				printHelp();
				return -1;
			}
		}
		currArg++;
	}
	if(!(bAnalyzeVideo||bAnalyzeCamera)){
		bAnalyzeCamera=true;
		cameraIDs=new int[1];
		cameraIDs[0]=0;
	}
	if(bAnalyzeCamera && InputFrameNum<3)
		InputFrameNum=3;
	cameraIDs[1];
	JobScheduler *jobScheduler=new JobFIFOQueue();
	jobScheduler->StartWork();
	if(bAnalyzeVideo){
		cSrcVideos=new PSTR[viewsNum];
		int size;
		for(int i=0;i<viewsNum;i++){
			size=WideCharToMultiByte(CP_ACP,0,szSrcVideo[i],-1,cSrcVideos[i],0,NULL,NULL);
			cSrcVideos[i]=new CHAR[size];
			WideCharToMultiByte(CP_ACP,0,szSrcVideo[i],-1,cSrcVideos[i],size,NULL,NULL);
		}
	}
	PDPC_Interface ui(jobScheduler);
	InputFrameNum=MIN(MAXINT,InputFrameNum);
	startIndex=MAX(0,startIndex);
	Job *job=new CaptureVideoJob(viewsNum,cSrcVideos,startIndex,InputFrameNum);
	jobScheduler->ScheduleNewJob(job);
	ui.InitializeInterfaceCallbacks();
	ui.StartInterfaceLoop();

	for(int i=0;i<viewsNum;i++)
		delete []cSrcVideos[i];
	delete[] cSrcVideos;
	delete []cameraIDs;
	return 0;
}

int parseCameraIDs(int ** IDs,_TCHAR* argv[],int index,int maxIndex){
	int num=0;
	int tempIndex=index;
	while(index<maxIndex&&iswdigit(argv[index++][0]))
		num++;
	index=tempIndex;
	if(num>0){
		*IDs=new int[num];
		for(int i=0;i<num;i++)
			(*IDs)[i]=_wtoi(argv[index++]);
	}
	return num;
}