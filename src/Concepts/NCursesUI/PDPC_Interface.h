#pragma once
#include "stdafx.h"
#include <curses.h>
#include <windows.h>
#include "PDPC_CommandParser.h"
#include "PDPC_CommandWorker.h"
#include "PDPC_WindowPanel.h"
#include "PDPC_CommandProcessor.h"
#include "PDPC_Job.h"
#define buffSize 256
#define KEY_ENTER2       0x00D  /* insert char or enter ins mode */
#define KEY_BACKSPACE2       0x008  /* insert char or enter ins mode */
class PDPC_Interface{
public:
	PDPC_Interface(JobScheduler *jobScheduler);
	~PDPC_Interface();
	int InitializeWindows();
	int InitializePanels();
	int InitializeInterfaceCallbacks();
	int StartInterfaceLoop();
	//HANDLE WaitForInterfaceThread();
private:
	JobScheduler *m_jobScheduler;

	DWORD m_dwThreadID;
	bool* m_continue;
	PDPC_CommandParser m_parser;
	PDPC_CommandWorker *m_worker;
	WINDOW *m_wUserInput;
	WINDOW *m_wStatusBar;
	WINDOW *m_wMenu;
	WINDOW *m_wMain;
	PDPC_WindowPanel* m_currentPanel;
	PDPC_WindowPanel* m_helpPanel;
	PDPC_WindowPanel* m_statisticPanel;
	int m_row,m_col;
};