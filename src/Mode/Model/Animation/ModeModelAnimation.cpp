/*
 * ModeModelAnimation.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelAnimation.h"
#include "ModeModelAnimationSkeleton.h"
#include "ModeModelAnimationVertex.h"
#include "../Mesh/ModeModelMeshTriangle.h"
#include "../Dialog/ModelAnimationDialog.h"

ModeModelAnimation *mode_model_animation = NULL;

ModeModelAnimation::ModeModelAnimation(Mode *_parent, DataModel *_data)
{
	name = "ModelAnimation";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_move");
	mode_model_animation_skeleton = new ModeModelAnimationSkeleton(this, data);
	mode_model_animation_vertex = new ModeModelAnimationVertex(this, data);
}


ModeModelAnimation::~ModeModelAnimation()
{
}



void ModeModelAnimation::OnCommand(const string & id)
{
}



void ModeModelAnimation::OnStart()
{
	dialog = new ModelAnimationDialog(ed, true, data);
	dialog->Update();

	data->UpdateAnimation();
	Subscribe(data);
	OnUpdate(data);
}


void ModeModelAnimation::OnUpdateMenu()
{
}



void ModeModelAnimation::OnEnd()
{
	Unsubscribe(data);
	delete(dialog);
}



void ModeModelAnimation::OnUpdate(Observable *o)
{
	if (data->move->Type == MoveTypeSkeletal)
		ed->SetMode(mode_model_animation_skeleton);
	else // vertex/none
		ed->SetMode(mode_model_animation_vertex);
}



void ModeModelAnimation::OnDrawWin(int win, irect dest)
{
}


