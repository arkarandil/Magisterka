#include "stdafx.h"
#include "ITask.h"
#include "ITaskScheduler.h"
#pragma once

struct TaskNode
{
	public:
	TaskNode(ITask* Task):_Task(Task),_next(NULL){};
	void AttachNext(TaskNode*node){_next=node;}
	void PerformAndClearTask()
	{
		_Task->Perform();
		delete _Task;
		_Task=NULL;
	}

	ITask* _Task;
	TaskNode * _next;
};

class PDPC_PROCESSINGCORE_API FIFOTaskQueue:public ITaskScheduler
{
public:
	virtual ~FIFOTaskQueue();
	FIFOTaskQueue();
	virtual int ScheduleNewTask(ITask** Task);
	virtual void StartWork(ITask** Task);
	virtual void StartWork();
	virtual void StopWork();
	friend DWORD WINAPI FIFOSchedulerThread(PVOID scheduler);
private:
	HANDLE _hQueueNotEmptyEvent;			// used in pausing capturing
	TaskNode* _head;
	TaskNode* _tail;
	CRITICAL_SECTION _cs;
	bool _continueWork;
};