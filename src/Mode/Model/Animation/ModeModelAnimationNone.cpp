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

void ModeModelAnimationNone::on_start()
{
	multi_view->clearData(data);

	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	on_update(data, "");
}

void ModeModelAnimationNone::on_end()
{
	multi_view->clearData(data);
}

void ModeModelAnimationNone::on_update(Observable* o, const string &message)
{
	mode_model_mesh->fill_selection_buffer(mode_model_animation->vertex);
}

void ModeModelAnimationNone::on_update_menu()
{
}

void ModeModelAnimationNone::on_draw_win(MultiView::Window *win)
{
	mode_model_mesh->on_draw_win(win);
}



