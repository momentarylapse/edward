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
}

void ModeModelMeshCreatePlane::onStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_plane_dialog", ed);
	dialog->setInt("np_num_x", HuiConfig.getInt("NewPlaneNumX", 4));
	dialog->setInt("np_num_y",HuiConfig.getInt("NewPlaneNumY", 4));
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->show();
	dialog->event("hui:close", this, &ModeModelMeshCreatePlane::onClose);

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreatePlane::onEnd()
{
	delete(dialog);
}


void ModeModelMeshCreatePlane::onLeftButtonUp()
{
	if (pos_chosen){
		int nx = dialog->getInt("np_num_x");
		int ny = dialog->getInt("np_num_y");
		HuiConfig.setInt("NewPlaneNumX", nx);
		HuiConfig.setInt("NewPlaneNumY", ny);

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



void ModeModelMeshCreatePlane::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos_chosen){
		vector n = length[0] ^ length[1];
		n.normalize();
		/// vertices
		vector a = pos;
		vector b = pos + length[0];
		vector c = pos + length[1];
		vector d = pos + length[0] + length[1];
		VBTemp->clear();
		VBTemp->addTria(a, -n, 0, 0, c, -n, 0, 0, d, -n, 0, 0);
		VBTemp->addTria(a, -n, 0, 0, d, -n, 0, 0, b, -n, 0, 0);
		VBTemp->addTria(b,  n, 0, 0, d,  n, 0, 0, c,  n, 0, 0);
		VBTemp->addTria(b,  n, 0, 0, c,  n, 0, 0, a,  n, 0, 0);
		mode_model->setMaterialCreation();
		NixDraw3D(VBTemp);
	}
}



void ModeModelMeshCreatePlane::onMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->getCursor3d();
		vector dir0, dir1, dir2;
		multi_view->mouse_win->getMovingFrame(dir0, dir1, dir2);
		length[0] = dir1 * VecDotProduct(dir1, pos2 - pos);
		length[1] = dir2 * VecDotProduct(dir2, pos2 - pos);
		invert = (((length[0] ^ length[1]) * dir0) > 0);
	}
}

void ModeModelMeshCreatePlane::onClose()
{
	abort();
}
