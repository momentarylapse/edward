/*
 * ModeModelSkeleton.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelSkeleton.h"


ModeModelSkeleton *mode_model_skeleton = NULL;


ModeModelSkeleton::ModeModelSkeleton(Mode *_parent, DataModel *_data)
{
	name = "ModelSkeleton";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}

ModeModelSkeleton::~ModeModelSkeleton()
{
}

void ModeModelSkeleton::OnMiddleButtonUp()
{
}



void ModeModelSkeleton::OnCommand(const string & id)
{
}



void ModeModelSkeleton::OnRightButtonDown()
{
}



void ModeModelSkeleton::OnKeyUp()
{
}



void ModeModelSkeleton::Draw()
{
}



void ModeModelSkeleton::OnMiddleButtonDown()
{
}



void ModeModelSkeleton::OnMouseMove()
{
}



void ModeModelSkeleton::OnUpdateMenu()
{
}



void ModeModelSkeleton::Start()
{
}


void ModeModelSkeleton::OnLeftButtonDown()
{
}



void ModeModelSkeleton::End()
{
}



void ModeModelSkeleton::OnKeyDown()
{
}



void ModeModelSkeleton::OnLeftButtonUp()
{
}



void ModeModelSkeleton::OnRightButtonUp()
{
}



void ModeModelSkeleton::OnUpdate(Observable *o)
{
}



void ModeModelSkeleton::DrawWin(int win, irect dest)
{
}


