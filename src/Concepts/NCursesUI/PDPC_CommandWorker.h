#include "stdafx.h"
#include "PDPC_Job.h"


#pragma once
DWORD WINAPI ShowSetThread(PVOID vname);

class PDPC_CommandWorker{
public:
	PDPC_CommandWorker(JobScheduler *jobScheduler):m_jobScheduler(jobScheduler){}
	void LoadScript(char** argv,int argc,void*);
	void StopInterface(char** argv,int argc,void*);
	void PrintCommand(char** argv,int argc,void*);
	void AddDialogBoxJob(char** argv,int argc,void*);
	void ShowImageSet(char** argv,int argc,void*);	
private:
	JobScheduler *m_jobScheduler;
};


