#include "stdafx.h"
#include <windows.h>
#include "PDPC_Job.h"
#include "PDPC_JobsSet.h"
#include <curses.h>
#include"PDPC_CommandWorker.h"
#include "PDPC_ImgDataManager.h"
#include"PDPC_CommandParser.h"

class LoadScriptJob:public Job{
public:
	LoadScriptJob(PDPC_CommandParser* parser,char* filePath):
	  m_parser(parser),m_filePath(filePath){}
	void Perform(){
		string line;
		ifstream myfile (m_filePath);
		if (myfile.is_open())
		{
			while ( myfile.good() )
			{
				getline (myfile,line);
				m_parser->ParseCommand((char*)line.c_str());
			}
			myfile.close();
		}
		free(m_filePath);
	}
private:
	PDPC_CommandParser* m_parser;
	char* m_filePath;
};


void PDPC_CommandWorker::StopInterface(char** argv,int argc,void* v){
	bool* continueWork=(bool*)v;
	*continueWork=false;
}

void PDPC_CommandWorker::PrintCommand(char** argv,int argc,void* v){
	WINDOW *wStatusBar=(WINDOW *)v;
	wmove(wStatusBar,0,0);
	wclrtoeol(wStatusBar);
	if(argc==0)
		wprintw(wStatusBar,"\0");
	else	
		for(int i=0;i<argc;i++){
			wprintw(wStatusBar,argv[0]);
			wprintw(wStatusBar,"next");
		}
	wrefresh(wStatusBar);
}
void PDPC_CommandWorker::AddDialogBoxJob(char** argv,int argc,void* v){
	int number=0;
	for(int i=0;i<argc;i++){
		number=atoi(argv[i]);
		Job*job=new DialogBoxJob(number);
		m_jobScheduler->ScheduleNewJob(job);
	}	
}



void PDPC_CommandWorker::LoadScript(char** argv,int argc,void*v){
	PDPC_CommandParser *parser=(PDPC_CommandParser *)v;
	Job *job;
	for(int i=0;i<argc;i++){
		job=new LoadScriptJob(parser,strdup(argv[i]));
		m_jobScheduler->ScheduleNewJob(job);
	}
}


void PDPC_CommandWorker::ShowImageSet(char** argv,int argc,void*){
	for(int i=0;i<argc;i++){
		char*name=strdup(argv[i]);
		chBEGINTHREADEX(NULL,0,&ShowSetThread,name,0,NULL); //starting thread
	}
}




DWORD WINAPI ShowSetThread(PVOID vname){
	char* name=(char*)vname;
	
	cvNamedWindow (name, CV_WINDOW_AUTOSIZE);
	
	bool continueShow=true;
	int index=0;
	int view=0;
	ImgSet* set=ImgDataManager::Instance().GetSetInfo(name);
	IplImage*im=ImgDataManager::Instance().GetImage(name,0,0);
	if(set==NULL)
		return -1;
	int key;
	
	while(continueShow){
		cvShowImage(name,im);
		key=cv::waitKey(0);
		switch(key){
			case 'q':
				continueShow=false;
				break;
			case 'n':
				if(index<set->m_imgNum-1){
					index++;
					cvReleaseImage(&im);
					im=ImgDataManager::Instance().GetImage(name,index,view);
				}
				break;
			case 'p':
				if(index>0){
					index--;
					cvReleaseImage(&im);
					im=ImgDataManager::Instance().GetImage(name,index,view);
				}
				break;
			default:
				if(key>='0'&&key<set->m_viewsInImg+'0'){
					view=key-'0';
					cvReleaseImage(&im);
					im=ImgDataManager::Instance().GetImage(name,index,view);
					
				}
				break;
		}
	}
	cvReleaseImage(&im);
	cvDestroyWindow(name);

	return 0;
}


