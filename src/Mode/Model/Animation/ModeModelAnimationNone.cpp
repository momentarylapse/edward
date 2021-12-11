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

ModeModelAnimationNone::ModeModelAnimationNone(ModeBase* _parent, MultiView::MultiView *mv) :
	Mode<DataModel>("ModelAnimationNone", _parent, mv, "menu_move")
{
}

void ModeModelAnimationNone::on_start() {
	multi_view->clear_data(data);

	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	mode_model_mesh->fill_selection_buffer(mode_model_animation->vertex);
}

void ModeModelAnimationNone::on_end() {
	multi_view->clear_data(data);
}

void ModeModelAnimationNone::on_draw_win(MultiView::Window *win) {
	mode_model_mesh->on_draw_win(win);
}



