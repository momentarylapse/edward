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
#include "../../../../lib/nix/nix.h"


namespace MultiView{
	float snap_f(MultiView *mv, float f);
	string format_length(MultiView *mv, float l);
}



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
	dialog->enable("x", true);
	dialog->enable("y", true);
	dialog->enable("complexity", false);
}

void ModeModelMeshCreateBall::onTypeSphere()
{
	dialog->enable("x", false);
	dialog->enable("y", false);
	dialog->enable("complexity", true);
}

void ModeModelMeshCreateBall::onTypePhysical()
{
	dialog->enable("x", false);
	dialog->enable("y", false);
	dialog->enable("complexity", false);
}

void ModeModelMeshCreateBall::onStart()
{
	// Dialog
	dialog = hui::CreateResourceDialog("new_ball_dialog", ed);

	dialog->setInt("x", hui::Config.getInt("NewBallNumX", 8));
	dialog->setInt("y",hui::Config.getInt("NewBallNumY", 16));
	dialog->setInt("complexity", hui::Config.getInt("NewBallComplexity", 8));
	bool sphere = hui::Config.getBool("NewBallSphere", false);
	dialog->check("type:ball", !sphere);
	dialog->check("type:sphere", sphere);
	dialog->enable("x", !sphere);
	dialog->enable("y", !sphere);
	dialog->enable("complexity", sphere);
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateBall::abort, this));
	dialog->event("type:ball", std::bind(&ModeModelMeshCreateBall::onTypeBall, this));
	dialog->event("type:sphere", std::bind(&ModeModelMeshCreateBall::onTypeSphere, this));
	dialog->event("type:physical", std::bind(&ModeModelMeshCreateBall::onTypePhysical, this));

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
		bool ball = dialog->isChecked("type:ball");
		bool sphere = dialog->isChecked("type:sphere");
		int nx = dialog->getInt("x");
		int ny = dialog->getInt("y");
		int complexity = dialog->getInt("complexity");
		hui::Config.setInt("NewBallNumX", nx);
		hui::Config.setInt("NewBallNumY", ny);
		hui::Config.setInt("NewBallComplexity", complexity);
		hui::Config.setBool("NewBallSphere", sphere);
		if (ball)
			geo = new GeometryBall(pos, radius, nx, ny);
		else if (sphere)
			geo = new GeometrySphere(pos, radius, complexity);
		else // physical
			geo = new GeometrySphere(pos, radius, 6);
	}
}



void ModeModelMeshCreateBall::onLeftButtonUp()
{
	if (pos_chosen){
		bool physical = dialog->isChecked("type:physical");
		if (physical){
			ModelVertex v;
			v.pos = pos;
			ModelBall b;
			b.index = data->skin[0].vertex.num;
			b.radius = radius;
			data->skin[0].vertex.add(v);
			data->ball.add(b);

		}else{
			data->pasteGeometry(*geo, mode_model_mesh->current_material);
			data->selectOnlySurface(&data->surface.back());
		}

		abort();
	}else{
		pos = multi_view->getCursor3d();
		message = _("Kugelradius: ");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreateBall::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos_chosen){
		mode_model->setMaterialCreation();
		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
	}
}


void ModeModelMeshCreateBall::onMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->getCursor3d(pos);
		radius = (pos2 - pos).length();
		if (multi_view->snap_to_grid)
			radius = MultiView::snap_f(multi_view, radius);
		updateGeometry();
		message = _("Kugelradius: ") + MultiView::format_length(multi_view, radius);
	}
}


