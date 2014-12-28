/*
 * ModeModelMeshCreateBall.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateBall.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryBall.h"
#include "../../../../Data/Model/Geometry/GeometrySphere.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"

ModeModelMeshCreateBall::ModeModelMeshCreateBall(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateBall", _parent)
{
	message = _("Kugelzentrum w&ahlen");

	pos_chosen = false;
	radius = 0;
	geo = NULL;
}

ModeModelMeshCreateBall::~ModeModelMeshCreateBall()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateBall::onTypeBall()
{
	dialog->enable("nb_x", true);
	dialog->enable("nb_y", true);
	dialog->enable("nb_complexity", false);
}

void ModeModelMeshCreateBall::onTypeSphere()
{
	dialog->enable("nb_x", false);
	dialog->enable("nb_y", false);
	dialog->enable("nb_complexity", true);
}

void ModeModelMeshCreateBall::onStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_ball_dialog", ed);

	dialog->setInt("nb_x", HuiConfig.getInt("NewBallNumX", 8));
	dialog->setInt("nb_y",HuiConfig.getInt("NewBallNumY", 16));
	dialog->setInt("nb_complexity", HuiConfig.getInt("NewBallComplexity", 8));
	bool sphere = HuiConfig.getBool("NewBallSphere", false);
	dialog->check("ball_type:ball", !sphere);
	dialog->check("ball_type:sphere", sphere);
	dialog->enable("nb_x", !sphere);
	dialog->enable("nb_y", !sphere);
	dialog->enable("nb_complexity", sphere);
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->show();
	dialog->eventS("hui:close", &HuiFuncIgnore);
	dialog->event("ball_type:ball", this, &ModeModelMeshCreateBall::onTypeBall);
	dialog->event("ball_type:sphere", this, &ModeModelMeshCreateBall::onTypeSphere);

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreateBall::onEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateBall::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		bool sphere = dialog->isChecked("ball_type:sphere");
		int nx = dialog->getInt("nb_x");
		int ny = dialog->getInt("nb_y");
		int complexity = dialog->getInt("nb_complexity");
		HuiConfig.setInt("NewBallNumX", nx);
		HuiConfig.setInt("NewBallNumY", ny);
		HuiConfig.setInt("NewBallComplexity", complexity);
		HuiConfig.setBool("NewBallSphere", sphere);
		if (sphere)
			geo = new GeometrySphere(pos, radius, complexity);
		else
			geo = new GeometryBall(pos, radius, nx, ny);
	}
}



void ModeModelMeshCreateBall::onLeftButtonUp()
{
	if (pos_chosen){
		data->pasteGeometry(*geo, mode_model_mesh->current_material);
		data->selectOnlySurface(&data->surface.back());

		abort();
	}else{
		pos = multi_view->getCursor3d();
		message = _("Kugel skalieren");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreateBall::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos_chosen){
		mode_model->setMaterialCreation();
		geo->preview(VBTemp);
		NixDraw3D(VBTemp);
	}
}



void ModeModelMeshCreateBall::onMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->getCursor3d(pos);
		radius = (pos2 - pos).length();
		updateGeometry();
	}
}


