/*
 * ModeModelAnimationNone.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationNone.h"
#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "../Mesh/ModeModelMeshPolygon.h"

ModeModelAnimationNone *mode_model_animation_none = NULL;

ModeModelAnimationNone::ModeModelAnimationNone(Mode* _parent, DataModel* _data) :
	Mode("ModelAnimationNone", _parent, _data, ed->multi_view_3d, "menu_move")
{
	data = _data;
}

ModeModelAnimationNone::~ModeModelAnimationNone()
{
}

void ModeModelAnimationNone::OnStart()
{
	multi_view->ResetData(NULL);
	multi_view->MVRectable = false;

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
	mode_model_mesh_polygon->FillSelectionBuffers();
}

void ModeModelAnimationNone::OnUpdateMenu()
{
}

void ModeModelAnimationNone::OnDrawWin(int win)
{
	mode_model_mesh_polygon->OnDrawWin(win);
}



