/*
 * ModeModelAnimationNone.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationNone.h"
#include "ModeModelAnimation.h"
#include "../../../EdwardWindow.h"
#include "../../../multiview/MultiView.h"
#include "../mesh/ModeModelMesh.h"
#include "../ModeModel.h"

ModeModelAnimationNone *mode_model_animation_none = NULL;

ModeModelAnimationNone::ModeModelAnimationNone(ModeModelAnimation* _parent, MultiView::MultiView *mv) :
	Mode<ModeModelAnimation, DataModel>(_parent->ed, "ModelAnimationNone", _parent, mv, "menu_move")
{
}

void ModeModelAnimationNone::on_start() {
	multi_view->clear_data(data);

	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	ed->mode_model->mode_model_mesh->fill_selection_buffer(mode_model_animation->vertex);
}

void ModeModelAnimationNone::on_end() {
	multi_view->clear_data(data);
}

void ModeModelAnimationNone::on_draw_win(MultiView::Window *win) {
	ed->mode_model->mode_model_mesh->on_draw_win(win);
}



