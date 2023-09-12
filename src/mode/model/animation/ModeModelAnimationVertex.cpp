/*
 * ModeModelAnimationVertex.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationVertex.h"
#include "ModeModelAnimation.h"
#include "../ModeModel.h"
#include "../skeleton/ModeModelSkeleton.h"
#include "../mesh/selection/MeshSelectionModePolygon.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../multiview/MultiView.h"

ModeModelAnimationVertex *mode_model_animation_vertex = NULL;

ModeModelAnimationVertex::ModeModelAnimationVertex(ModeModelAnimation* _parent, MultiView::MultiView *mv) :
	Mode<ModeModelAnimation, DataModel>(_parent->session, "ModelAnimationVertex", _parent, mv, "menu_move")
{
	mouse_action = -1;
}

void ModeModelAnimationVertex::on_start() {
	session->win->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-animation-vertex-toolbar");

	session->mode_model->allow_selection_modes(true);

	chooseMouseFunction(MultiView::ACTION_SELECT);

	data->out_changed >> create_sink([this] { on_data_change(); });
	multi_view->out_selection_changed >> create_sink([this]{ session->mode_model->mode_model_mesh->selection_mode->on_update_selection(); });
	on_data_change();
}

void ModeModelAnimationVertex::on_end() {
	data->unsubscribe(this);
	multi_view->unsubscribe(this);
	data->mesh->set_show_vertices(data->mesh->vertex);
	session->mode_model->mode_model_mesh->fill_selection_buffer(data->mesh->vertex);
}

void ModeModelAnimationVertex::on_command(const string& id) {
	if (id == "select")
		chooseMouseFunction(MultiView::ACTION_SELECT);
	if (id == "translate")
		chooseMouseFunction(MultiView::ACTION_MOVE);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ACTION_ROTATE);
	if (id == "scale")
		chooseMouseFunction(MultiView::ACTION_SCALE);
	if (id == "mirror")
		chooseMouseFunction(MultiView::ACTION_MIRROR);
}

void ModeModelAnimationVertex::chooseMouseFunction(int f) {
	mouse_action = f;

	// mouse action
	multi_view->set_mouse_action("ActionModelAnimationTransformVertices", mouse_action, false);
}

void ModeModelAnimationVertex::on_data_change() {
	data->mesh->set_show_vertices(parent->vertex);
	session->mode_model->mode_model_mesh->selection_mode->update_multi_view();
}

void ModeModelAnimationVertex::on_update_menu() {
	session->win->check("select", mouse_action == MultiView::ACTION_SELECT);
	session->win->check("translate", mouse_action == MultiView::ACTION_MOVE);
	session->win->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	session->win->check("scale", mouse_action == MultiView::ACTION_SCALE);
	session->win->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
}

void ModeModelAnimationVertex::on_draw_win(MultiView::Window *win) {
	session->mode_model->mode_model_mesh->draw_polygons(win, data->mesh, parent->vertex);

	session->mode_model->mode_model_skeleton->draw_skeleton(win, data->bone, true);

	session->mode_model->mode_model_mesh->draw_selection(win);

//	draw_edges(win, data->edit_mesh, data->edit_mesh->vertex, !selection_mode_edge->is_active());

	if (session->mode_model->mode_model_mesh->allow_draw_hover)
		session->mode_model->mode_model_mesh->selection_mode->on_draw_win(win);

}


