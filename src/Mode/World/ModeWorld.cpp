/*
 * ModeWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeWorld.h"
#include "../../Data/World/DataWorld.h"

ModeWorld *mode_world = NULL;

ModeWorld::ModeWorld()
{
	name = "World";
	parent = NULL;

	menu = HuiCreateResourceMenu("menu_world");
	data = new DataWorld;
	multi_view = ed->multi_view_3d;
	Subscribe(data);
}

ModeWorld::~ModeWorld()
{
	// TODO Auto-generated destructor stub
}

bool ModeWorld::SaveAs()
{
	return false;
}



void ModeWorld::OnCommand(const string & id)
{
}



void ModeWorld::OnLeftButtonDown()
{
}



bool ModeWorld::Save()
{
	return false;
}



void ModeWorld::OnMiddleButtonUp()
{
}



void ModeWorld::OnMouseMove()
{
}



void ModeWorld::OnUpdate(Observable *o)
{
}



void ModeWorld::OnKeyDown()
{
}



void ModeWorld::OnMiddleButtonDown()
{
}



void ModeWorld::OnRightButtonUp()
{
}



void ModeWorld::New()
{
	data->Reset();
	multi_view->Reset();
	ed->SetMode(mode_world);
}



void ModeWorld::OnLeftButtonUp()
{
}



void ModeWorld::Draw()
{
}



void ModeWorld::OnKeyUp()
{
}



void ModeWorld::End()
{
}



void ModeWorld::DrawWin(int win, irect dest)
{
}



void ModeWorld::Start()
{
	string dir = HuiAppDirectoryStatic + SysFileName("Data/icons/toolbar/");
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->ToolbarAddItem(L("new"),L("new"),dir + "new.png","new");
	ed->ToolbarAddItem(L("open"),L("open"),dir + "open.png","open");
	ed->ToolbarAddItem(L("save"),L("save"),dir + "save.png","save");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(L("undo"),L("undo"),dir + "undo.png","undo");
	ed->ToolbarAddItem(L("redo"),L("redo"),dir + "redo.png","redo");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Push"),_("ViewStage Push"),dir + "view_push.png","view_push");
	ed->ToolbarAddItem(_("Pop"),_("ViewStage Pop"),dir + "view_pop.png","view_pop");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Eigenschaften"),_("Eigenschaften"), dir + "configure.png", "selection_properties");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
}



void ModeWorld::OnRightButtonDown()
{
}



void ModeWorld::OnUpdateMenu()
{
}



bool ModeWorld::Open()
{
	data->Reset();
	multi_view->Reset();
	ed->SetMode(mode_world);
	return false;
}


