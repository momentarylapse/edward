/*
 * ModeModelMeshBevelEdges.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ModeModelMeshBevelEdges.h"
#include "../ModeModelMesh.h"
#include "../../../../Action/Model/Mesh/Edge/ActionModelBevelEdges.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshBevelEdges::ModeModelMeshBevelEdges(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshBevelEdges", _parent)
{
	message = _("Scale radius [left click = done]");


	selection = data->get_selection();

	// find maximal radius
	rad_max = -1;
	for (ModelEdge &e: data->edge)
		if ((data->vertex[e.vertex[0]].is_selected) or (data->vertex[e.vertex[1]].is_selected)){
		float l = (data->vertex[e.vertex[0]].pos - data->vertex[e.vertex[1]].pos).length();
		if ((data->vertex[e.vertex[0]].is_selected) and (data->vertex[e.vertex[1]].is_selected))
			l /= 2;
		if ((l < rad_max) or (rad_max < 0))
			rad_max = l;
	}

	radius = rad_max / 4;

	if (!data->action_manager->preview(new ActionModelBevelEdges(radius)))
		abort();
}

void ModeModelMeshBevelEdges::on_start()
{
	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);
}

void ModeModelMeshBevelEdges::on_end()
{
	data->action_manager->clear_preview();
}

void ModeModelMeshBevelEdges::on_mouse_move()
{
	data->action_manager->clear_preview();

	radius += (hui::GetEvent()->dx) / multi_view->active_win->zoom();
	radius = clampf(radius, rad_max * 0.001f, rad_max);

	data->set_selection(selection);
	if (!data->action_manager->preview(new ActionModelBevelEdges(radius)))
		abort();

	message = _("Scale radius [left click = done]   ") + multi_view->format_length(radius);
}

void ModeModelMeshBevelEdges::on_left_button_down()
{
	data->action_manager->clear_preview();

	data->set_selection(selection);
	data->bevelSelectedEdges(radius);
	abort();
}

void ModeModelMeshBevelEdges::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);
}


