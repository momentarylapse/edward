/*
 * ModeModelMeshSplitPolygon.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ModeModelMeshSplitPolygon.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Polygon/ActionModelSplitPolygon.h"
#include "../../../../Action/Model/Mesh/Edge/ActionModelSplitEdge.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../MultiView/ColorScheme.h"
#include "../../../../lib/nix/nix.h"


ModeModelMeshSplitPolygon::ModeModelMeshSplitPolygon(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshSplitPolygon", _parent) {}

void ModeModelMeshSplitPolygon::on_start() {
	polygon = -1;
	factor = 0;
	edge = -1;

	message = _("place new vertex into polygon");

	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	ed->mode_model->allow_selection_modes(false);

	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);
}

void ModeModelMeshSplitPolygon::on_left_button_up() {
	/*triangle = multi_view->MouseOver;
	surface = multi_view->MouseOverSet;
	pos = multi_view->MouseOverTP;*/

	if (polygon >= 0) {
		if (edge >= 0)
			data->execute(new ActionModelSplitEdge(edge, factor));
		else
			data->execute(new ActionModelSplitPolygon(polygon, pos));
		//Abort();
		polygon = -1;
		multi_view->force_redraw();
	}
}

void ModeModelMeshSplitPolygon::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	polygon = multi_view->hover.index;
	pos = multi_view->hover.point;
	edge = -1;

	if (polygon >= 0) {
		vector pp = win->project(pos);
		pp.z = 0;
		auto &poly = data->mesh->polygon[polygon];
		Array<vector> v, p;
		for (int k=0;k<poly.side.num;k++) {
			v.add(data->mesh->vertex[poly.side[k].vertex].pos);
			p.add(win->project(v[k]));
			p.back().z = 0;
		}

		// close to an edge?
		for (int k=0;k<v.num;k++) {
			if (VecLineDistance(pp, p[k], p[(k + 1) % v.num]) < 10) {
				float f = (pp - p[k]).length() / (p[k] - p[(k + 1) % v.num]).length();
				pos = v[k] * (1 - f) + v[(k + 1) % v.num] * f;
				pp = win->project(pos);
				edge = poly.side[k].edge;
				factor = f;
			}
		}


		nix::set_z(false, false);
		set_color(scheme.CREATION_LINE);
		set_line_width(scheme.LINE_WIDTH_MEDIUM);
		for (int k=0;k<v.num;k++)
			draw_line(v[k], pos);
		nix::set_z(true, true);
	}
}
