/*
 * ModeModelMeshCreatePlane.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "ModeModelMeshCreatePlane.h"
#include "../../../../Data/Model/Geometry/GeometryPlane.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshCreatePlane::ModeModelMeshCreatePlane(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreatePlane", _parent)
{
	message = _("Ebene: erster Punkt");
	pos_chosen = false;
	length[0] = length[1] = v_0;
	invert = false;
}

void ModeModelMeshCreatePlane::on_start()
{
	// Dialog
	dialog = hui::CreateResourceDialog("new_plane_dialog", ed);
	dialog->set_int("np_num_x", hui::Config.get_int("NewPlaneNumX", 4));
	dialog->set_int("np_num_y",hui::Config.get_int("NewPlaneNumY", 4));
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreatePlane::onClose, this));

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreatePlane::on_end()
{
	delete(dialog);
}


void ModeModelMeshCreatePlane::on_left_button_up()
{
	if (pos_chosen){
		int nx = dialog->get_int("np_num_x");
		int ny = dialog->get_int("np_num_y");
		hui::Config.set_int("NewPlaneNumX", nx);
		hui::Config.set_int("NewPlaneNumY", ny);

		if (invert){
			pos += length[0];
			length[0] = -length[0];
		}

		GeometryPlane pl = GeometryPlane(pos, length[0], length[1], nx, ny);
		data->pasteGeometry(pl, mode_model_mesh->current_material);
		data->selectOnlySurface(&data->surface.back());

		abort();
	}else{
		pos = multi_view->getCursor3d();
		message = _("Ebene: zweiter Punkt");
		pos_chosen = true;
	}
}



void ModeModelMeshCreatePlane::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	if (pos_chosen){
		vector n = length[0] ^ length[1];
		n.normalize();
		/// vertices
		vector a = pos;
		vector b = pos + length[0];
		vector c = pos + length[1];
		vector d = pos + length[0] + length[1];
		nix::vb_temp->clear();
		nix::vb_temp->addTria(a, -n, 0, 0, c, -n, 0, 0, d, -n, 0, 0);
		nix::vb_temp->addTria(a, -n, 0, 0, d, -n, 0, 0, b, -n, 0, 0);
		nix::vb_temp->addTria(b,  n, 0, 0, d,  n, 0, 0, c,  n, 0, 0);
		nix::vb_temp->addTria(b,  n, 0, 0, c,  n, 0, 0, a,  n, 0, 0);
		mode_model->set_material_creation();
		nix::Draw3D(nix::vb_temp);
	}
}



void ModeModelMeshCreatePlane::on_mouse_move()
{
	if (pos_chosen){
		vector pos2 = multi_view->getCursor3d();
		vector dir0, dir1, dir2;
		multi_view->mouse_win->getMovingFrame(dir0, dir1, dir2);
		length[0] = dir1 * vector::dot(dir1, pos2 - pos);
		length[1] = dir2 * vector::dot(dir2, pos2 - pos);
		invert = (((length[0] ^ length[1]) * dir0) > 0);
	}
}

void ModeModelMeshCreatePlane::onClose()
{
	abort();
}
