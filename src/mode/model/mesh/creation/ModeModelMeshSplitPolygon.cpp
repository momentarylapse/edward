/*
 * ModeModelMeshSplitPolygon.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ModeModelMeshSplitPolygon.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include "../../../../Session.h"
#include "../../../../action/model/mesh/polygon/ActionModelSplitPolygon.h"
#include "../../../../action/model/mesh/edge/ActionModelSplitEdge.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../multiview/ColorScheme.h"
#include "../../../../lib/nix/nix.h"


ModeModelMeshSplitPolygon::ModeModelMeshSplitPolygon(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshSplitPolygon", _parent) {}

void ModeModelMeshSplitPolygon::on_start() {
	polygon = -1;
	factor = 0;
	edge = -1;

	message = _("place new vertex into polygon");

	parent->set_selection_mode(parent->selection_mode_polygon);
	session->mode_model->allow_selection_modes(false);

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
		vec3 pp = win->project(pos);
		pp.z = 0;
		auto &poly = data->mesh->polygon[polygon];
		Array<vec3> v, p;
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


#if HAS_LIB_GL
		nix::set_z(false, false);
		win->drawing_helper->set_color(scheme.CREATION_LINE);
		win->drawing_helper->set_line_width(scheme.LINE_WIDTH_MEDIUM);
		for (int k=0;k<v.num;k++)
			win->drawing_helper->draw_line(v[k], pos);
		nix::set_z(true, true);
#endif
	}
}
