/*
 * ModeModelAnimation.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelAnimation.h"

ModeModelAnimation *mode_model_animation = NULL;

ModeModelAnimation::ModeModelAnimation(Mode *_parent, DataModel *_data)
{
	name = "ModelSkeleton";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}


ModeModelAnimation::~ModeModelAnimation()
{
}

void ModeModelAnimation::Draw()
{
}



void ModeModelAnimation::OnCommand(const string & id)
{
}



void ModeModelAnimation::Start()
{
}


void ModeModelAnimation::OnUpdateMenu()
{
}



void ModeModelAnimation::End()
{
}



void ModeModelAnimation::OnUpdate(Observable *o)
{
}



void ModeModelAnimation::DrawWin(int win, irect dest)
{
}

