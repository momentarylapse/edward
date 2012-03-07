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
	parent = NULL;

	menu = HuiCreateResourceMenu("menu_font");
	data = new DataFont;
	multi_view = ed->multi_view_2d;
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



void ModeFont::End()
{
}



void ModeFont::Draw()
{
}



bool ModeFont::SaveAs()
{
}



void ModeFont::OnKeyUp()
{
}



void ModeFont::OnMiddleButtonUp()
{
}



bool ModeFont::Save()
{
}



void ModeFont::OnCommand(const string & id)
{
}



bool ModeFont::Open()
{
	data->Reset();
	multi_view->Reset();
	ed->SetMode(mode_font);
}



void ModeFont::OnRightButtonUp()
{
}



void ModeFont::New()
{
	data->Reset();
	multi_view->Reset();
	ed->SetMode(mode_font);
}



void ModeFont::Start()
{
}



void ModeFont::OnMouseMove()
{
}



void ModeFont::OnDataChange()
{
}



void ModeFont::OnLeftButtonUp()
{
}



void ModeFont::OnRightButtonDown()
{
}



void ModeFont::DrawWin(int win, irect dest)
{
}



void ModeFont::OnMiddleButtonDown()
{
}


