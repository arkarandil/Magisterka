#include "stdafx.h"
#pragma once

class PDPC_PROCESSINGCORE_API ITask
{
public:

	virtual ~ITask(){};
	/*
	Task to be performed
	*/
	virtual void Perform()=0;
};