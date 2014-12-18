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
#include "../Mesh/ModeModelMesh.h"

ModeModelAnimationNone *mode_model_animation_none = NULL;

ModeModelAnimationNone::ModeModelAnimationNone(ModeBase* _parent) :
	Mode<DataModel>("ModelAnimationNone", _parent, ed->multi_view_3d, "menu_move")
{
}

ModeModelAnimationNone::~ModeModelAnimationNone()
{
}

void ModeModelAnimationNone::onStart()
{
	multi_view->ClearData(NULL);
	multi_view->allow_rect = false;

	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	onUpdate(data, "");
}

void ModeModelAnimationNone::onEnd()
{
	multi_view->ClearData(NULL);
}

void ModeModelAnimationNone::onCommand(const string& id)
{
}

void ModeModelAnimationNone::onUpdate(Observable* o, const string &message)
{
	mode_model_mesh->fillSelectionBuffers(mode_model_animation->vertex);
}

void ModeModelAnimationNone::onUpdateMenu()
{
}

void ModeModelAnimationNone::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh->onDrawWin(win);
}



