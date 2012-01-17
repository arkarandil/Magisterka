#include "stdafx.h"
#include <curses.h>
#include <windows.h>
#include<tchar.h>
#include<Shlwapi.h>
#include<Strsafe.h>
#include"PDPC_WindowPanel.h"

PDPC_WindowPanel::PDPC_WindowPanel(WINDOW* targetWindow,int height,int width,int xoff,int yoff){
	m_wTargetWindow=targetWindow;
	m_height=height;
	m_width=width;
	m_XOffset=xoff;
	m_YOffset=yoff;
	m_properitesNum=0;
	m_selectedIndex=0;
	m_topIndex=0;
	m_allocatedMem=0;
	m_content=NULL;
}
PDPC_WindowPanel::~PDPC_WindowPanel(){
	if(m_content!=NULL)
	{
		for(int i=0;i<m_properitesNum;i++)
			delete m_content[i];
		delete [] m_content;
	}
}
int PDPC_WindowPanel::AddProperty(bool editable,char *name,char* type,char* value){
	int length;
	if(m_properitesNum>=m_allocatedMem)
		AllocatePropertySet(m_allocatedMem*2);
	PropertyDesc*desc=(PropertyDesc*) malloc (sizeof(PropertyDesc));
	m_content[m_properitesNum]=desc;
	m_content[m_properitesNum]->m_editable=editable;
	length=min(PROPETY_NAME_MAX_L-1,strlen(name)+1);
	StringCchCopyA(m_content[m_properitesNum]->m_name,length,name);
	m_content[m_properitesNum]->m_name[PROPETY_NAME_MAX_L-1]='\0';

	length=min(PROPETY_TYPE_MAX_L-1,strlen(type)+1);
	StringCchCopyA(m_content[m_properitesNum]->m_type,length,type);
	m_content[m_properitesNum]->m_type[PROPETY_TYPE_MAX_L-1]='\0';

	length=min(PROPETY_VALUE_MAX_L-1,strlen(value)+1);
	StringCchCopyA(m_content[m_properitesNum]->m_value,length,value);
	m_content[m_properitesNum]->m_value[PROPETY_VALUE_MAX_L-1]='\0';
	m_properitesNum++;
	return 0;
}
int PDPC_WindowPanel::AllocatePropertySet(int num){
	if(m_content==NULL){
		
		m_content= (PropertyDesc**) calloc (num,sizeof(PropertyDesc*));
		if(m_content==NULL)
			return NULL;
		m_allocatedMem=num;
	}
	else if(num>m_allocatedMem){
		PropertyDesc** temp;
		temp= (PropertyDesc**) calloc (num,sizeof(PropertyDesc*));
		for(int i=0;i<m_properitesNum;i++){
			temp[i]= m_content[i];
		}
		delete [] m_content;
		m_content=temp;
		m_allocatedMem=num;
	}
	


}
void PDPC_WindowPanel::FillWindow(bool onlySelected){
	int row=m_YOffset;
	int startIndex;
	int endIndex;
	if(onlySelected){
		startIndex=max(0,m_selectedIndex-1);
		endIndex=min(m_selectedIndex+1,m_properitesNum-1);
		row+=startIndex-m_topIndex;
	}else{
		wclear(m_wTargetWindow);

		box(m_wTargetWindow, '|' , '-');
		startIndex=m_topIndex;
		endIndex=min(m_topIndex+m_height-1,m_topIndex+m_properitesNum-1);
	}
	for(startIndex;startIndex<=endIndex;startIndex++){
		wmove(m_wTargetWindow,row,m_XOffset);
		//wclrtoeol(m_wTargetWindow);
		if(startIndex==m_selectedIndex)
			wattron(m_wTargetWindow,A_BOLD);
		wprintw(m_wTargetWindow,m_content[startIndex]->m_name);
		wmove(m_wTargetWindow,row,PROPETY_NAME_MAX_L+1+m_XOffset);
		wprintw(m_wTargetWindow,m_content[startIndex]->m_value);
		if(startIndex==m_selectedIndex)
			wattroff(m_wTargetWindow,A_BOLD);
		row++;
	}
	wrefresh(m_wTargetWindow);
}
void PDPC_WindowPanel::MoveUp(){
	if(m_selectedIndex>0){
		if(m_topIndex>0 && m_selectedIndex<=m_topIndex+2){
			m_topIndex--;
			m_selectedIndex--;
			FillWindow(false);
		}
		else{
			m_selectedIndex--;
			FillWindow(true);
		}
	}
}
void PDPC_WindowPanel::MoveDown(){
	if(m_topIndex<max(1,m_properitesNum-m_height) && m_selectedIndex>=m_topIndex+m_height-2){
		m_topIndex++;
		m_selectedIndex++;
		FillWindow(false);
	}
	else if(m_selectedIndex<m_properitesNum-1){
		m_selectedIndex++;
		FillWindow(true);
	}
	
}
void PDPC_WindowPanel::ResetSelection(){
	m_selectedIndex=0;
	m_topIndex=0;
}