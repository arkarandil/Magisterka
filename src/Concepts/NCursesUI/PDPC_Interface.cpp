#include "stdafx.h"
#include <curses.h>

#include "PDPC_Interface.h"
#include "PDPC_CommandParser.h"
#include "PDPC_CommandWorker.h"

PDPC_Interface::PDPC_Interface(JobScheduler *jobScheduler){
	m_jobScheduler=jobScheduler;
	 initscr();				/* start the curses mode */
	 getmaxyx(stdscr,m_row,m_col);
	raw();				/* Line buffering disabled	*/
	
	noecho();			/* Don't echo() while we do getch */
	cbreak();
	m_continue=new bool;
	*m_continue=true;
	m_worker=new PDPC_CommandWorker(m_jobScheduler);
	m_parser.SetWorker(m_worker);
	start_color();
	InitializeWindows();
	InitializePanels();

}
int PDPC_Interface::InitializeWindows(){
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_WHITE);
	init_pair(3, COLOR_BLUE, COLOR_WHITE);
	init_pair(4, COLOR_BLACK, COLOR_WHITE);
	//curs_set(0);
	m_wUserInput = newwin(1, m_col, m_row-1, 0);
	m_wStatusBar= newwin(1, m_col, m_row-2, 0);
	m_wMenu= newwin(1, m_col, m_row-3, 0);
	m_wMain= newwin(m_row-3, m_col, 0, 0);
	
	wbkgd(m_wUserInput,COLOR_PAIR(1));
	wbkgd(m_wStatusBar,COLOR_PAIR(1));
	wbkgd(m_wMenu,COLOR_PAIR(3));
	wbkgd(m_wMain,COLOR_PAIR(4));

	//clearok(stdscr,TRUE);
	wmove(m_wUserInput,0,0);
	wclrtoeol(m_wUserInput);
	box(m_wMain, '|' , '-');
	wprintw(m_wMenu,"F1-Help");
	wmove(m_wMenu,0,m_col/3-7);
	wprintw(m_wMenu,"F2-Statistics");
	wmove(m_wMenu,0,2*m_col/3-6);
	wprintw(m_wMenu,"F3-Settings");
	wmove(m_wMenu,0,m_col-7);
	wprintw(m_wMenu,"F4-Edit");
	wrefresh(m_wUserInput);
	wrefresh(m_wStatusBar);
	wrefresh(m_wMenu);
	wrefresh(m_wMain);
	keypad(m_wUserInput, TRUE);		/* We get F1, F2 etc..		*/
	return 0;
}
int PDPC_Interface::InitializePanels(){
	m_helpPanel=new PDPC_WindowPanel(m_wMain,m_row-5,m_col-2,1,1);
	m_helpPanel->AllocatePropertySet(2);
	m_helpPanel->AddProperty(false,"exit","n","Exit program, doesn't save work");
	m_helpPanel->AddProperty(false,"set \"property\" \"value\"","n","Set property to new value");
	m_helpPanel->AddProperty(false,"start","n","Start work");


	m_statisticPanel=new PDPC_WindowPanel(m_wMain,m_row-5,m_col-2,1,1);
	m_statisticPanel->AllocatePropertySet(4);
	char buff[6];
	for(int i=0;i<64;i++){
		_itoa((100*i)%131,buff,10);
		m_statisticPanel->AddProperty(false,"start","n",buff);
	}

	m_currentPanel=m_helpPanel;
	m_currentPanel->FillWindow(false);
	return 0;
}
PDPC_Interface::~PDPC_Interface(){
	endwin();
	delete m_worker;
	delete m_continue;
	delwin(m_wMenu);
	delwin(m_wMain);
	delwin(m_wUserInput);
	delwin(m_wStatusBar);
	delete m_helpPanel;
	delete m_statisticPanel;
}

int PDPC_Interface::InitializeInterfaceCallbacks(){
	m_parser.AddCallback("load",COMMAND(&PDPC_CommandWorker::LoadScript,&m_parser));
	m_parser.AddCallback("box",COMMAND(&PDPC_CommandWorker::AddDialogBoxJob,NULL));
	m_parser.AddCallback("exit",COMMAND(&PDPC_CommandWorker::StopInterface,m_continue));
	m_parser.AddCallback("print",COMMAND(&PDPC_CommandWorker::PrintCommand,m_wStatusBar));
	m_parser.AddCallback("show",COMMAND(&PDPC_CommandWorker::ShowImageSet,NULL));
	return 0;
}
int PDPC_Interface::StartInterfaceLoop(){
	
	
	int ch=0;
	char buffer[buffSize];
	
	int index;
 	while(*m_continue){
		wmove(m_wUserInput,0,0);
		wclrtoeol(m_wUserInput);
		index=0;
		buffer[0]='\0';
		ch=0;
		
		while(ch!=KEY_ENTER2){
			ch = wgetch(m_wUserInput);
			if(ch<=255&&isprint(ch)&&index<buffSize-1){
				winsch(m_wUserInput,ch);
				buffer[index++]=ch;
				wmove(m_wUserInput,0,index);
				buffer[index]='\0';
			}
			else if(ch==KEY_UP){
				m_currentPanel->MoveUp();
			}
			else if(ch==KEY_DOWN){
				m_currentPanel->MoveDown();
			}
			else if(ch==KEY_F(1)){
				if(m_currentPanel!=m_helpPanel){
					m_currentPanel=m_helpPanel;
					m_currentPanel->ResetSelection();
					m_currentPanel->FillWindow(false);

				}
			}
			else if(ch==KEY_F(2)){
				if(m_currentPanel!=m_statisticPanel){
					m_currentPanel=m_statisticPanel;
					m_currentPanel->ResetSelection();
					m_currentPanel->FillWindow(false);
				}
			}
			else if(ch==KEY_BACKSPACE2 && index>0){
				buffer[--index]='\0';
				wmove(m_wUserInput,0,index);
				wdelch(m_wUserInput);
			
			}
			wrefresh(m_wUserInput);
		}

		if(strlen(buffer)>0)
			m_parser.ParseCommand(buffer);
		
		
	}
	
	return 0;
}