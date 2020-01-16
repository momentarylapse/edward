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
#include "../../../../lib/nix/nix.h"

namespace MultiView{
	float snap_f(MultiView *mv, float f);
	string format_length(MultiView *mv, float l);
}

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

void ModeModelMeshCreateCylinder::on_start()
{
	dialog = hui::CreateResourceDialog("new_cylinder_dialog",ed);

	dialog->set_int("rings", hui::Config.get_int("NewCylinderRings", 4));
	dialog->set_int("edges", hui::Config.get_int("NewCylinderEdges", 8));
	dialog->check("round", hui::Config.get_bool("NewCylinderRound", false));
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateCylinder::onClose, this));
	dialog->event("type:visible", std::bind(&ModeModelMeshCreateCylinder::onTypeVisible, this));
	dialog->event("type:physical", std::bind(&ModeModelMeshCreateCylinder::onTypePhysical, this));

	dialog->check("type:visible", true);

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreateCylinder::on_end()
{
	delete(dialog);
}

void ModeModelMeshCreateCylinder::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos.num == 2){
		//bool physical = dialog->isChecked("type:physical");
		bool round = dialog->is_checked("round");
		int rings = dialog->get_int("rings");
		int edges = dialog->get_int("edges");
		hui::Config.set_int("NewCylinderRings", rings);
		hui::Config.set_int("NewCylinderEdges", edges);
		hui::Config.set_bool("NewCylinderRound", round);

		Array<float> r = {radius, radius};
		geo = new GeometryCylinder(pos, r, rings, edges, round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT);
	}
}

void ModeModelMeshCreateCylinder::on_mouse_move()
{
	if (pos.num == 2){
		vector p = multi_view->getCursor3d(pos.back());
		radius = (p - pos.back()).length();
		if (multi_view->snap_to_grid)
			radius = MultiView::snap_f(multi_view, radius);
		float min_rad = 10 / multi_view->active_win->zoom(); // 10 px
		if (radius < min_rad)
			radius = min_rad;
		updateGeometry();
		message = _("Zylinderradius: ") + MultiView::format_length(multi_view, radius);
	}
}



void ModeModelMeshCreateCylinder::on_left_button_up()
{
	if (pos.num == 2){
		bool physical = dialog->is_checked("type:physical");

		if (physical){
			ModelCylinder c;
			c.index[0] = data->skin[0].vertex.num;
			c.index[1] = data->skin[0].vertex.num + 1;
			c.radius = radius;
			c.round = dialog->is_checked("round");
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

void ModeModelMeshCreateCylinder::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	if (pos.num > 0){

		// control points
		nix::SetColor(multi_view->ColorCreationLine);
		nix::SetShader(nix::default_shader_2d);
		for (int i=0;i<pos.num;i++){
			vector pp = win->project(pos[i]);
			nix::DrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}

		//mode_model->setMaterialCreation(2);

		// control polygon
		MultiView::set_wide_lines(2);
		if (pos.num == 2)
			nix::DrawLine3D(pos[0], pos[1]);
		else
			nix::DrawLine3D(pos[0], multi_view->getCursor3d());
		//nix::SetColor(White);
	}
	if (pos.num == 2){
		mode_model->set_material_creation();
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
