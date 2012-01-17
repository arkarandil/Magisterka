#include "stdafx.h"
#include "ITask.h"
#pragma once

class PDPC_PROCESSINGCORE_API ITaskScheduler
{
public:
	virtual ~ITaskScheduler(){};
	/*
	dwd
	*/
	virtual int  ScheduleNewTask(ITask** Task)=0;
	virtual void StartWork()=0;
	/*
	Start working schedular
	initialize work with Task
	*/
	virtual void StartWork(ITask** Task)=0;
	virtual void StopWork()=0;
};