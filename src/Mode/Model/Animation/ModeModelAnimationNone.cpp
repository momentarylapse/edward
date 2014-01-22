/*
 * ModeModelAnimationNone.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationNone.h"
#include "ModeModelAnimation.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../Mesh/ModeModelMeshPolygon.h"

ModeModelAnimationNone *mode_model_animation_none = NULL;

ModeModelAnimationNone::ModeModelAnimationNone(ModeBase* _parent) :
	Mode<DataModel>("ModelAnimationNone", _parent, ed->multi_view_3d, "menu_move")
{
}

ModeModelAnimationNone::~ModeModelAnimationNone()
{
}

void ModeModelAnimationNone::OnStart()
{
	multi_view->ResetData(NULL);
	multi_view->allow_rect = false;

	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	OnUpdate(data);
}

void ModeModelAnimationNone::OnEnd()
{
	multi_view->ResetData(NULL);
}

void ModeModelAnimationNone::OnCommand(const string& id)
{
}

void ModeModelAnimationNone::OnUpdate(Observable* o)
{
	mode_model_mesh_polygon->FillSelectionBuffers(mode_model_animation->vertex);
}

void ModeModelAnimationNone::OnUpdateMenu()
{
}

void ModeModelAnimationNone::OnDrawWin(MultiView::Window *win)
{
	mode_model_mesh_polygon->OnDrawWin(win);
}



