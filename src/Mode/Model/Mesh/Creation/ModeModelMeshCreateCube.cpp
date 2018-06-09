/*
 * ModeModelMeshCreateCube.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "ModeModelMeshCreateCube.h"
#include "../../../../Data/Model/Geometry/GeometryCube.h"
#include "../../../../Data/Model/Geometry/GeometryPlane.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshCreateCube::ModeModelMeshCreateCube(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCube", _parent)
{
	message = _("W&urfel: Punkt 1 / 3");
	pos_chosen = false;
	pos2_chosen = false;
	for (int i=0;i<3;i++)
		length[i] = v_0;
	geo = NULL;
}

ModeModelMeshCreateCube::~ModeModelMeshCreateCube()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateCube::onClose()
{
	abort();
}

void ModeModelMeshCreateCube::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos2_chosen){
		int num_1 = dialog->getInt("nc_x");
		int num_2 = dialog->getInt("nc_y");
		int num_3 = dialog->getInt("nc_z");
		hui::Config.setInt("NewCubeNumX", num_1);
		hui::Config.setInt("NewCubeNumY", num_2);
		hui::Config.setInt("NewCubeNumZ", num_3);

		geo = new GeometryCube(pos, length[0], length[1], length[2], num_1, num_2, num_3);
	}else{
		geo = new GeometryPlane(pos, length[0], length[1], 1, 1);
	}
}


void set_dpos3(vector *length, const vector &dpos)
{
	vector n = length[0] ^ length[1];
	n.normalize();
	length[2] = n * (n * dpos);
	float min_thick = 10 / ed->multi_view_3d->active_win->zoom(); // 10 px
	if (length[2].length() < min_thick)
		length[2] = n * min_thick;
}



void ModeModelMeshCreateCube::onLeftButtonUp()
{
	if (pos_chosen){
		if (pos2_chosen){

			data->pasteGeometry(*geo, mode_model_mesh->current_material);
			data->selectOnlySurface(&data->surface.back());

			abort();
		}else{
			pos2 = multi_view->getCursor3d();
			message = _("W&urfel: Punkt 3 / 3");
			pos2_chosen = true;
			set_dpos3(length, v_0);
			updateGeometry();
		}
	}else{
		pos = multi_view->getCursor3d();
		message = _("W&urfel: Punkt 2 / 3");
		pos_chosen = true;
		updateGeometry();
	}
}

void ModeModelMeshCreateCube::onMouseMove()
{
	if (pos_chosen){
		if (!pos2_chosen){
			vector pos2 = multi_view->getCursor3d();
			vector dir0 = multi_view->mouse_win->getDirectionRight();
			vector dir1 = multi_view->mouse_win->getDirectionUp();
			length[0] = dir0 * VecDotProduct(dir0, pos2 - pos);
			length[1] = dir1 * VecDotProduct(dir1, pos2 - pos);
			updateGeometry();
		}else{
			set_dpos3(length, multi_view->getCursor3d() - pos);
			updateGeometry();
		}
	}
}



void ModeModelMeshCreateCube::onStart()
{
	// Dialog
	dialog = hui::CreateResourceDialog("new_cube_dialog", ed);

	dialog->setInt("nc_x", hui::Config.getInt("NewCubeNumX", 1));
	dialog->setInt("nc_y", hui::Config.getInt("NewCubeNumY", 1));
	dialog->setInt("nc_z", hui::Config.getInt("NewCubeNumZ", 1));
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateCube::onClose, this));

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}

void ModeModelMeshCreateCube::onEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateCube::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	mode_model->setMaterialCreation();
	if (pos_chosen){
		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
	}
}


