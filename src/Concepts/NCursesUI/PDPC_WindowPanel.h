#pragma once
#include "stdafx.h"
#include <curses.h>
#include <windows.h>
#define PROPETY_NAME_MAX_L 32
#define PROPETY_TYPE_MAX_L 16
#define PROPETY_VALUE_MAX_L 32
struct PropertyDesc{
public:
	bool m_editable;
	char m_name[PROPETY_NAME_MAX_L];
	char m_type[PROPETY_TYPE_MAX_L];
	char m_value[PROPETY_VALUE_MAX_L];
};
class PDPC_WindowPanel{
public:
	PDPC_WindowPanel(WINDOW* TargetWindow,int height,int width,int xoff=0,int yoff=0);
	~PDPC_WindowPanel();
	int AddProperty(bool editable,char *name,char* type,char* value);
	int AllocatePropertySet(int num);
	void FillWindow(bool onlySelected);
	void MoveUp();
	void MoveDown();
	void ResetSelection();
private:
	WINDOW* m_wTargetWindow;
	int m_height;
	int m_width;
	PropertyDesc** m_content;
	int m_selectedIndex;
	int m_topIndex;
	int m_properitesNum;
	int m_allocatedMem;
	int m_XOffset;
	int m_YOffset;
};