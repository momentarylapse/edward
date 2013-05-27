/*
 * ModeModelAnimation.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelAnimation.h"
#include "ModeModelAnimationNone.h"
#include "ModeModelAnimationSkeleton.h"
#include "ModeModelAnimationVertex.h"
#include "../Mesh/ModeModelMeshPolygon.h"
#include "../Dialog/ModelAnimationDialog.h"

ModeModelAnimation *mode_model_animation = NULL;

ModeModelAnimation::ModeModelAnimation(Mode *_parent, DataModel *_data) :
	Mode("ModelAnimation", _parent, _data, NULL, "menu_move")
{
	data = _data;
	mode_model_animation_none = new ModeModelAnimationNone(this, data);
	mode_model_animation_skeleton = new ModeModelAnimationSkeleton(this, data);
	mode_model_animation_vertex = new ModeModelAnimationVertex(this, data);
}


ModeModelAnimation::~ModeModelAnimation()
{
}



void ModeModelAnimation::OnCommand(const string & id)
{
	if (id == "move_frame_inc")
		data->SetCurrentFrameNext();
	if (id == "move_frame_dec")
		data->SetCurrentFramePrevious();
	if (id == "move_frame_delete")
		data->AnimationDeleteCurrentFrame();
	if (id == "move_frame_insert")
		data->AnimationDuplicateCurrentFrame();
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
	else if (data->move->Type == MoveTypeVertex)
		ed->SetMode(mode_model_animation_vertex);
	else
		ed->SetMode(mode_model_animation_none);
}



void ModeModelAnimation::OnDrawWin(MultiViewWindow *win)
{
}


