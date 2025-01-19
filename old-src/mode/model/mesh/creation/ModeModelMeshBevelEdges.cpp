/*
 * ModeModelMeshBevelEdges.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ModeModelMeshBevelEdges.h"
#include "../ModeModelMesh.h"
#include "../../../../action/model/mesh/edge/ActionModelBevelEdges.h"
#include "../../ModeModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../EdwardWindow.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"

ModeModelMeshBevelEdges::ModeModelMeshBevelEdges(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshBevelEdges", _parent) {}

void ModeModelMeshBevelEdges::on_start() {
	message = _("Scale radius [left click = done]");


	selection = data->mesh->get_selection();

	// find maximal radius
	rad_max = -1;
	for (ModelEdge &e: data->mesh->edge)
		if ((data->mesh->vertex[e.vertex[0]].is_selected) or (data->mesh->vertex[e.vertex[1]].is_selected)){
			float l = (data->mesh->vertex[e.vertex[0]].pos - data->mesh->vertex[e.vertex[1]].pos).length();
			if ((data->mesh->vertex[e.vertex[0]].is_selected) and (data->mesh->vertex[e.vertex[1]].is_selected))
				l /= 2;
			if ((l < rad_max) or (rad_max < 0))
				rad_max = l;
		}

	radius = rad_max / 4;

	if (!data->action_manager->preview(new ActionModelBevelEdges(radius, parent->current_material)))
		abort();

	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);
}

void ModeModelMeshBevelEdges::on_end() {
	data->action_manager->clear_preview();
}

void ModeModelMeshBevelEdges::on_mouse_move() {
	data->action_manager->clear_preview();

	radius += (hui::get_event()->d.x) / multi_view->active_win->zoom();
	radius = clamp(radius, rad_max * 0.001f, rad_max);

	data->set_selection(selection);
	if (!data->action_manager->preview(new ActionModelBevelEdges(radius, parent->current_material)))
		abort();

	message = _("Scale radius [left click = done]   ") + multi_view->format_length(radius);
}

void ModeModelMeshBevelEdges::on_left_button_down() {
	data->action_manager->clear_preview();

	data->set_selection(selection);
	data->bevelSelectedEdges(radius);
	abort();
}

void ModeModelMeshBevelEdges::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}


