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
}

ModeWorld::~ModeWorld()
{
	// TODO Auto-generated destructor stub
}

bool ModeWorld::SaveAs()
{
}



void ModeWorld::OnCommand(const string & id)
{
}



void ModeWorld::OnLeftButtonDown()
{
}



bool ModeWorld::Save()
{
}



void ModeWorld::OnMiddleButtonUp()
{
}



void ModeWorld::OnMouseMove()
{
}



void ModeWorld::OnDataChange()
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
}



void ModeWorld::OnRightButtonDown()
{
}



bool ModeWorld::Open()
{
	data->Reset();
	multi_view->Reset();
	ed->SetMode(mode_world);
}


