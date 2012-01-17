#include "stdafx.h"
#include <map>
#include "Tools.h"
#include"PDPC_CommandWorker.h"
#pragma once



typedef void (PDPC_CommandWorker::*COMMAND_CALLBACK)(char** argv,int argc,void *);
using namespace std;
struct COMMAND{
	COMMAND():m_function(NULL),m_object(NULL){};
	COMMAND(COMMAND_CALLBACK f,void  *v):m_function(f),m_object(v){};
	COMMAND_CALLBACK m_function;
	void* m_object;
};

class PDPC_CommandParser{
public:
	PDPC_CommandParser(){}
	PDPC_CommandParser(PDPC_CommandWorker* work):m_worker(work){}
	void SetWorker(PDPC_CommandWorker* work){m_worker=work;}
	int AddCallback(char*,COMMAND);
	int ParseCommand(char * buff);
private:
	PDPC_CommandWorker* m_worker;
	map<char*,COMMAND,cmp_str> m_commands;
};