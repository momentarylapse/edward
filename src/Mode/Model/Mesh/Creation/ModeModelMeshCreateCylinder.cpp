/*
 * ModeModelMeshCreateCylinder.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinder.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryCylinder.h"
#include "../../../../Action/Model/Mesh/Physical/ActionModelAddCylinder.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../MultiView/ColorScheme.h"
#include "../../../../lib/nix/nix.h"



ModeModelMeshCreateCylinder::ModeModelMeshCreateCylinder(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCylinder", _parent)
{
	message = _("Cylinder: starting point");

	radius = 0;
	geo = NULL;
}

ModeModelMeshCreateCylinder::~ModeModelMeshCreateCylinder() {
	if (geo)
		delete geo;
}

void ModeModelMeshCreateCylinder::on_start() {
	dialog = new hui::Panel();
	dialog->from_resource("new_cylinder_dialog");

	dialog->set_int("rings", hui::config.get_int("NewCylinderRings", 4));
	dialog->set_int("edges", hui::config.get_int("NewCylinderEdges", 8));
	dialog->check("round", hui::config.get_bool("NewCylinderRound", false));

	ed->set_side_panel(dialog);

	bool physical = (mode_model_mesh->current_skin == MESH_PHYSICAL);
	if (physical) {
		dialog->enable("rings", false);
		dialog->enable("edges", false);
	}

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	ed->activate("");
}


void ModeModelMeshCreateCylinder::on_end() {
	ed->set_side_panel(nullptr);
}

void ModeModelMeshCreateCylinder::update_geometry() {
	if (geo)
		delete geo;
	if (pos.num == 2){
		//bool physical = dialog->isChecked("type:physical");
		bool round = dialog->is_checked("round");
		int rings = dialog->get_int("rings");
		int edges = dialog->get_int("edges");
		hui::config.set_int("NewCylinderRings", rings);
		hui::config.set_int("NewCylinderEdges", edges);
		hui::config.set_bool("NewCylinderRound", round);

		Array<float> r = {radius, radius};
		geo = new GeometryCylinder(pos, r, rings, edges, round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT);
	}
}

void ModeModelMeshCreateCylinder::on_mouse_move() {
	if (pos.num == 2) {
		vec3 p = multi_view->get_cursor(pos.back());
		radius = (p - pos.back()).length();
		if (multi_view->snap_to_grid)
			radius = multi_view->snap_f(radius);
		float min_rad = 10 / multi_view->active_win->zoom(); // 10 px
		if (radius < min_rad)
			radius = min_rad;
		update_geometry();
		message = _("Cylinder radius: ") + multi_view->format_length(radius);
	} else if (pos.num == 1) {
		message = _("Cylinder length: ") + multi_view->format_length((multi_view->get_cursor() - pos[0]).length());
	}
}



void ModeModelMeshCreateCylinder::on_left_button_up() {
	if (pos.num == 2) {
		bool physical = (mode_model_mesh->current_skin == MESH_PHYSICAL);

		if (physical) {
			ModelCylinder c;
			c.index[0] = data->phys_mesh->vertex.num;
			c.index[1] = data->phys_mesh->vertex.num + 1;
			c.radius = radius;
			c.round = dialog->is_checked("round");
			data->execute(new ActionModelAddCylinder(c));

			data->phys_mesh->vertex.add(ModelVertex(pos[0]));
			data->phys_mesh->vertex.add(ModelVertex(pos[1]));

		} else {
			data->pasteGeometry(*geo, mode_model_mesh->current_material);
		}

		abort();
	} else {
		pos.add(multi_view->get_cursor());

		if (pos.num > 1) {
			//OnMouseMove();
			message = _("Cylinder: radius");
			update_geometry();
			//ed->ForceRedraw();
		} else {
			message = _("Cylinder: ...points");
		}
	}
}

void ModeModelMeshCreateCylinder::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (pos.num > 0) {

		// control points
		set_color(scheme.CREATION_LINE);
		nix::set_shader(nix::Shader::default_2d);
		for (int i=0;i<pos.num;i++) {
			vec3 pp = win->project(pos[i]);
			draw_rect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}

		//mode_model->setMaterialCreation(2);

		// control polygon
		set_line_width(scheme.LINE_WIDTH_MEDIUM);
		set_color(scheme.CREATION_LINE);
		if (pos.num == 2)
			draw_line(pos[0], pos[1]);
		else
			draw_line(pos[0], multi_view->get_cursor());
	}
	if (pos.num == 2) {
		set_material_creation();
		geo->build(nix::vb_temp);
		nix::draw_triangles(nix::vb_temp);

		if (win == multi_view->mouse_win)
			draw_helper_line(win, pos[1], multi_view->get_cursor());
	}
}
