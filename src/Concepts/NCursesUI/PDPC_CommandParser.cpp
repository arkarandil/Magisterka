#include "stdafx.h"
#include"PDPC_CommandParser.h"

int PDPC_CommandParser::AddCallback(char* command,COMMAND comm){
	m_commands.insert(map<char*,COMMAND>::value_type(command,comm));
	return 0;
}

int PDPC_CommandParser::ParseCommand(char * buffer){
	char *buffCopy;
	char* cCommand;
	int argc;
	char**argv;
	buffCopy= strdup(buffer);
	cCommand = strtok (buffer," \t\0");

	argc=0;
	while(strtok (NULL," \t"))
		argc++;

	argv=new char* [argc];
	cCommand = strtok (buffCopy," \t\0");
	for(int i=0;i<argc;i++){
		argv[i]=strtok (NULL," \t\0");
	}
		
	COMMAND command;
	map<char*,COMMAND,cmp_str>::iterator it;
	it=m_commands.find(cCommand);
	if(it==m_commands.end())
		return -1;
	command=(COMMAND)it->second;
	((*m_worker).*(command.m_function))(argv,argc,command.m_object);
	free(buffCopy);
	delete[]argv;
	return 0;
}