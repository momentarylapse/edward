/*
 * ModeModelMeshCreatePolygon.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreatePolygon.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../EdwardWindow.h"
#include "../../../../Session.h"
#include "../../../../action/model/mesh/polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../multiview/ColorScheme.h"


ModeModelMeshCreatePolygon::ModeModelMeshCreatePolygon(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshCreatePolygon", _parent)
{
	message = format(_("Select polygon: %d -> [Ctrl + Return]"), 0);

	parent->set_selection_mode(parent->selection_mode_vertex);
	session->mode_model->allow_selection_modes(false);
}


void ModeModelMeshCreatePolygon::on_start() {
	for (ModelVertex &v: data->edit_mesh->vertex)
		v.is_special = false;

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}



void ModeModelMeshCreatePolygon::on_end() {
	for (ModelVertex &v: data->edit_mesh->vertex)
		v.is_special = false;
}


void ModeModelMeshCreatePolygon::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	win->drawing_helper->set_color(scheme.CREATION_LINE);
	win->drawing_helper->set_line_width(scheme.LINE_WIDTH_MEDIUM);
	for (int i=1;i<selection.num;i++) {
		vec3 pa = data->edit_mesh->vertex[selection[i - 1]].pos;
		vec3 pb = data->edit_mesh->vertex[selection[i    ]].pos;
		win->drawing_helper->draw_line(pa, pb);
	}
	if (selection.num > 0) {
		if (multi_view->hover.index >= 0)
			win->drawing_helper->draw_line(data->edit_mesh->vertex[selection.back()].pos, data->edit_mesh->vertex[multi_view->hover.index].pos);
		else
			win->drawing_helper->draw_line(data->edit_mesh->vertex[selection.back()].pos, multi_view->get_cursor());
	}
}



void ModeModelMeshCreatePolygon::on_command(const string &id) {
	if (id == "finish-action") {
		data->execute(new ActionModelAddPolygonAutoSkin(selection, parent->current_material));
		abort();
	}
}


void ModeModelMeshCreatePolygon::on_left_button_down() {
	if (multi_view->hover.index >= 0) {
		// closed loop -> done
		if (selection.num > 0)
			if (multi_view->hover.index == selection[0]) {
				data->execute(new ActionModelAddPolygonAutoSkin(selection, parent->current_material));
				abort();
				return;
			}

		// consistency?
		foreachi(int s, selection, i)
			if (s == multi_view->hover.index)
				if (i > 0) {
					session->set_message(_("No double points allowed!"));
					return;
				}

		// all ok -> add
		selection.add(multi_view->hover.index);

	} else {
		data->addVertex(multi_view->get_cursor());
		selection.add(data->edit_mesh->vertex.num - 1);
	}
	data->edit_mesh->vertex[selection.back()].is_special = true;
	message = format(_("Choose polygon: %d -> [Ctrl + Return]"), selection.num);
}

