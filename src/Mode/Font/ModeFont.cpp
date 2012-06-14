/*
 * ModeFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeFont.h"
#include "../../Data/Font/DataFont.h"
#include "../../lib/x/x.h"

ModeFont *mode_font = NULL;

ModeFont::ModeFont()
{
	name = "Font";

	menu = HuiCreateResourceMenu("menu_font");
	data = new DataFont;
	multi_view = ed->multi_view_2d;
	Subscribe(data);
}

ModeFont::~ModeFont()
{
}

void ModeFont::OnKeyDown()
{
}



void ModeFont::OnLeftButtonDown()
{
}



void ModeFont::OnEnd()
{
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
}



void ModeFont::OnDraw()
{
}



bool ModeFont::SaveAs()
{
	return false;
}



void ModeFont::OnKeyUp()
{
}



void ModeFont::OnMiddleButtonUp()
{
}



bool ModeFont::Save()
{
	return false;
}



void ModeFont::OnCommand(const string & id)
{
}



bool ModeFont::Open()
{
	data->Reset();
	multi_view->ResetView();
	ed->SetMode(mode_font);
	return false;
}



void ModeFont::OnRightButtonUp()
{
}



void ModeFont::New()
{
	data->Reset();
	multi_view->ResetView();
	ed->SetMode(mode_font);
}



void ModeFont::OnStart()
{
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
}



void ModeFont::OnMouseMove()
{
}



void ModeFont::OnUpdate(Observable *o)
{
}



void ModeFont::OnLeftButtonUp()
{
}



void ModeFont::OnRightButtonDown()
{
}



void ModeFont::OnDrawWin(int win, irect dest)
{
}



void ModeFont::OnMiddleButtonDown()
{
}



void ModeFont::OnUpdateMenu()
{
}


