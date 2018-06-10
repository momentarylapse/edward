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
	multi_view->clearData(data);

	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	onUpdate(data, "");
}

void ModeModelAnimationNone::onEnd()
{
	multi_view->clearData(data);
}

void ModeModelAnimationNone::onUpdate(Observable* o, const string &message)
{
	mode_model_mesh->fillSelectionBuffer(mode_model_animation->vertex);
}

void ModeModelAnimationNone::onUpdateMenu()
{
}

void ModeModelAnimationNone::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh->onDrawWin(win);
}



