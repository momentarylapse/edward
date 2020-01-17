/*
 * ModeModelMeshCreateTorus.cpp
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateTorus.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryTorus.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"

#define RADIUS_FACTOR	0.5f



ModeModelMeshCreateTorus::ModeModelMeshCreateTorus(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateTorus", _parent)
{
	message = _("Toruszentrum w&ahlen");

	pos_chosen = false;
	rad_chosen = false;
	radius1 = 0;
	radius2 = 0;
	axis = vector::EZ;
	geo = NULL;
}

ModeModelMeshCreateTorus::~ModeModelMeshCreateTorus()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateTorus::on_start()
{
	// Dialog
	dialog = hui::CreateResourceDialog("new_torus_dialog", ed);

	dialog->set_int("nt_rings", hui::Config.get_int("NewTorusNumX", 32));
	dialog->set_int("nt_edges", hui::Config.get_int("NewTorusNumY", 16));
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateTorus::onClose, this));

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	ed->activate("");
}


void ModeModelMeshCreateTorus::on_end()
{
	delete(dialog);
}

void ModeModelMeshCreateTorus::onClose()
{
	abort();
}

void ModeModelMeshCreateTorus::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		int nx = dialog->get_int("nt_rings");
		int ny = dialog->get_int("nt_edges");
		hui::Config.set_int("NewTorusNumX", nx);
		hui::Config.set_int("NewTorusNumY", ny);
		geo = new GeometryTorus(pos, axis, radius1, radius2, nx, ny);
	}
}


void ModeModelMeshCreateTorus::on_left_button_up()
{
	if (pos_chosen){
		if (rad_chosen){

			data->pasteGeometry(*geo, mode_model_mesh->current_material);
			data->selectOnlySurface(&data->surface.back());

			abort();
		}else{
			message = _("Torus innen skalieren");
			rad_chosen = true;
		}
	}else{
		pos = multi_view->get_cursor();
		message = _("Torus au&sen skalieren");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreateTorus::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (pos_chosen) {
		mode_model->set_material_creation();
		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
		nix::EnableLighting(false);
		ed->draw_str(100, 100, format("%.3f / %.3f", radius1, radius2));

		if (win == multi_view->mouse_win) {
			draw_helper_line(win, pos, multi_view->get_cursor());
		}
	}
}



void ModeModelMeshCreateTorus::on_mouse_move()
{
	axis = multi_view->mouse_win->getDirection();
	if (pos_chosen){
		vector m = multi_view->get_cursor(pos);
		if (rad_chosen){
			radius2 = (m - pos).length() * RADIUS_FACTOR;
			radius2 = multi_view->maybe_snap_f(radius2);
			message = _("Torus au&sen skalieren: ") + multi_view->format_length(radius1) + " / " + multi_view->format_length(radius2);
		}else{
			radius1 = (m - pos).length();
			radius2 = radius1 * RADIUS_FACTOR;
			radius1 = multi_view->maybe_snap_f(radius1);
			radius2 = multi_view->maybe_snap_f(radius2);

			message = _("Torus innen skalieren: ") + multi_view->format_length(radius1);
		}
		updateGeometry();
	}
}


