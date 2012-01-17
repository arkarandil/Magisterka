#include "stdafx.h"
#include "ITask.h"
#include "FIFOTaskQueue.h"
#include <iostream>
#include "Tools.h"
#include <assert.h>
FIFOTaskQueue:: FIFOTaskQueue()
{
	_hQueueNotEmptyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_continueWork=false;
	_head=NULL;
	_tail=NULL;
	InitializeCriticalSection(&_cs);
}

FIFOTaskQueue::~FIFOTaskQueue()
{
	EnterCriticalSection(&(_cs));
	TaskNode*tempNode;
	while(_head!=NULL)
	{
		tempNode=_head->_next;
		delete _head->_Task;
		_head->_Task=NULL;

		delete _head;
		_head=tempNode;
	}
	LeaveCriticalSection(&(_cs));
}

DWORD WINAPI FIFOSchedulerThread(PVOID scheduler)
{
	FIFOTaskQueue* sched=(FIFOTaskQueue*)scheduler;
	TaskNode*node;
	while(sched->_continueWork)
	{
		EnterCriticalSection(&(sched->_cs));
			node=sched->_head;
		LeaveCriticalSection(&(sched->_cs));
		while(node==NULL)		//Waiting when queue is empty
		{
			WaitForSingleObject(sched->_hQueueNotEmptyEvent, 10000);	//Signal from StopWork
			if(!sched->_continueWork)
				break;
			EnterCriticalSection(&(sched->_cs));
				node=sched->_head;
			LeaveCriticalSection(&(sched->_cs));
		}
		if(!sched->_continueWork)
				break;
		node->PerformAndClearTask();

		EnterCriticalSection(&(sched->_cs));
			sched->_head=node->_next;	//Removing finished node from queue
			delete node;
			if(sched->_head==NULL)
			{
				sched->_tail=NULL;
				ResetEvent(sched->_hQueueNotEmptyEvent);
			}
		LeaveCriticalSection(&(sched->_cs));
	}
	return 0;
}

void FIFOTaskQueue::StopWork()
{
	_continueWork=false;
	SetEvent(_hQueueNotEmptyEvent);
}

void FIFOTaskQueue::StartWork()
{
	EnterCriticalSection(&(_cs));
	if(!_continueWork)
	{
		_continueWork=true;
		chBEGINTHREADEX(NULL,0,&FIFOSchedulerThread,this,0,NULL); //starting thread
	}
	LeaveCriticalSection(&(_cs));
}

int FIFOTaskQueue::ScheduleNewTask(ITask** Task)
{
	assert(*Task!=NULL);
	TaskNode *node=new TaskNode(*Task);
	EnterCriticalSection(&_cs);
	if(_tail==NULL)
	{
		_head=_tail=node;
	}
	else
	{
		_tail->AttachNext(node);
		_tail=node;
	}
	LeaveCriticalSection(&_cs);
	SetEvent(_hQueueNotEmptyEvent);
	*Task=NULL;
	return 0;
}

void FIFOTaskQueue::StartWork(ITask** Task)
{
	assert(*Task!=NULL);
	ScheduleNewTask(Task);
	*Task=NULL;
	StartWork();
}