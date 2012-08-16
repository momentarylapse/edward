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
	multi_view = ed->multi_view_3d;
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
}


void ModeModelAnimation::OnUpdateMenu()
{
}



void ModeModelAnimation::OnEnd()
{
	delete(dialog);
}



void ModeModelAnimation::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
	}else if (o->GetName() == "MultiView"){
	}
}



void ModeModelAnimation::OnDrawWin(int win, irect dest)
{
	mode_model_mesh_triangle->DrawTrias();
}


