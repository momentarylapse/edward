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
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshCreateCylinder::ModeModelMeshCreateCylinder(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCylinder", _parent)
{
	message = _("Zylinder: Startpunkt");

	radius = 0;
	geo = NULL;
}

ModeModelMeshCreateCylinder::~ModeModelMeshCreateCylinder()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateCylinder::onStart()
{
	dialog = hui::CreateResourceDialog("new_cylinder_dialog",ed);

	dialog->setInt("rings", hui::Config.getInt("NewCylinderRings", 4));
	dialog->setInt("edges", hui::Config.getInt("NewCylinderEdges", 8));
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateCylinder::onClose, this));
	dialog->event("type:visible", std::bind(&ModeModelMeshCreateCylinder::onTypeVisible, this));
	dialog->event("type:physical", std::bind(&ModeModelMeshCreateCylinder::onTypePhysical, this));

	dialog->check("type:visible", true);

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreateCylinder::onEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateCylinder::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos.num == 2){
		//bool physical = dialog->isChecked("type:physical");
		bool round = dialog->isChecked("round");
		int rings = dialog->getInt("rings");
		int edges = dialog->getInt("edges");
		hui::Config.setInt("NewCylinderRings", rings);
		hui::Config.setInt("NewCylinderEdges", edges);

		Array<float> r = radius;
		r += radius;
		geo = new GeometryCylinder(pos, r, rings, edges, round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT);
	}
}

void ModeModelMeshCreateCylinder::onMouseMove()
{
	if (pos.num == 2){
		vector p = multi_view->getCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->active_win->zoom(); // 10 px
		if (radius < min_rad)
			radius = min_rad;
		updateGeometry();
	}
}



void ModeModelMeshCreateCylinder::onLeftButtonUp()
{
	if (pos.num == 2){
		bool physical = dialog->isChecked("type:physical");

		if (physical){
			ModelCylinder c;
			c.index[0] = data->skin[0].vertex.num;
			c.index[1] = data->skin[0].vertex.num + 1;
			c.radius = radius;
			c.round = dialog->isChecked("rout");
			data->cylinder.add(c);

			data->skin[0].vertex.add(ModelVertex(pos[0]));
			data->skin[0].vertex.add(ModelVertex(pos[1]));

		}else{
			data->pasteGeometry(*geo, mode_model_mesh->current_material);
			data->selectOnlySurface(&data->surface.back());
		}

		abort();
	}else{
		pos.add(multi_view->getCursor3d());

		if (pos.num > 1){
			//OnMouseMove();
			message = _("Zylinder: Radius");
			updateGeometry();
			//ed->ForceRedraw();
		}else{
			message = _("Zylinder: Endpunkt");
		}
	}
}

namespace MultiView{
extern nix::Shader* shader_lines_3d_colored;
};

void ModeModelMeshCreateCylinder::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos.num > 0){
		mode_model->setMaterialCreation(2);

		//nix::EnableLighting(false);
		nix::SetColor(Green);
		// control polygon
		nix::SetShader(MultiView::shader_lines_3d_colored);
		for (int i=0;i<pos.num;i++){
			vector pp = win->project(pos[i]);
			nix::DrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}
		nix::SetShader(nix::default_shader_3d);
		mode_model->setMaterialCreation(2);

		if (pos.num == 2)
			nix::DrawLine3D(pos[0], pos[1]);
		else
			nix::DrawLine3D(pos[0], multi_view->getCursor3d());
		//nix::SetColor(White);
	}
	if (pos.num == 2){
		nix::EnableLighting(true);
		mode_model->setMaterialCreation();
		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
	}
}

void ModeModelMeshCreateCylinder::onTypePhysical()
{
	dialog->enable("rings", false);
	dialog->enable("edges", false);
}

void ModeModelMeshCreateCylinder::onTypeVisible()
{
	dialog->enable("rings", true);
	dialog->enable("edges", true);
}

void ModeModelMeshCreateCylinder::onClose()
{
	abort();
}
