/*
 * ModeModelMeshCreateCylinderSnake.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinderSnake.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryCylinder.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

const float CYLINDER_CLOSING_DISTANCE = 20;


ModeModelMeshCreateCylinderSnake::ModeModelMeshCreateCylinderSnake(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCylinderSnake", _parent)
{
	message = _("Zylinderschlange... Punkte + Shift Return");

	radius = 0;
	closed = false;
	ready_for_scaling = false;
	geo = NULL;
}

void ModeModelMeshCreateCylinderSnake::onStart()
{
	dialog = hui::CreateResourceDialog("new_cylinder_dialog",ed);

	dialog->setInt("rings", hui::Config.getInt("NewCylinderRings", 4));
	dialog->setInt("edges", hui::Config.getInt("NewCylinderEdges", 8));
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateCylinderSnake::onClose, this));

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreateCylinderSnake::onEnd()
{
	delete(dialog);
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateCylinderSnake::updateGeometry()
{
	if (geo)
		delete(geo);
	if (ready_for_scaling){
		int rings = dialog->getInt("rings");
		int edges = dialog->getInt("edges");
		hui::Config.setInt("NewCylinderRings", rings);
		hui::Config.setInt("NewCylinderEdges", edges);

		geo = new GeometryCylinder(pos, radius, rings * (pos.num - 1), edges, closed ? GeometryCylinder::END_FLAT : GeometryCylinder::END_LOOP);
	}
}


void ModeModelMeshCreateCylinderSnake::onMouseMove()
{
	if (ready_for_scaling){
		vector p = multi_view->getCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->active_win->zoom(); // 10 px
		if (radius < min_rad)
			radius = min_rad;
		updateGeometry();
	}
}



void ModeModelMeshCreateCylinderSnake::onLeftButtonUp()
{
	if (ready_for_scaling){

		data->pasteGeometry(*geo, mode_model_mesh->current_material);
		data->selectOnlySurface(&data->surface.back());

		abort();
	}else{
		if (pos.num > 2){
			vector pp = multi_view->mouse_win->project(pos[0]);
			pp.z = 0;
			if ((pp - multi_view->m).length_fuzzy() < CYLINDER_CLOSING_DISTANCE){
				closed = true;
				ready_for_scaling = true;
				onMouseMove();
				message = _("Zylinder: Radius");
				updateGeometry();
				ed->forceRedraw();
				return;
			}

		}
		pos.add(multi_view->getCursor3d());
	}
}



void ModeModelMeshCreateCylinderSnake::onKeyDown(int k)
{
	if (k == hui::KEY_SHIFT + hui::KEY_RETURN){
		if (pos.num > 1){
			ready_for_scaling = true;
			onMouseMove();
			message = _("Zylinder: Radius");
			updateGeometry();
			ed->forceRedraw();
		}
	}
}





void ModeModelMeshCreateCylinderSnake::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos.num > 0){
		nix::EnableLighting(false);
		// control polygon
		for (int i=0;i<pos.num;i++){
			vector pp = win->project(pos[i]);
			nix::SetColor(Green);
			nix::DrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
			nix::SetColor(White);
			if (i > 0)
				nix::DrawLine3D(pos[i - 1], pos[i]);
		}

		// spline curve
		Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
		for (vector &p: pos)
			inter.add(p);
		if (!ready_for_scaling)
			inter.add(multi_view->getCursor3d());
		inter.normalize();
		nix::SetColor(Green);
		for (int i=0;i<100;i++)
			nix::DrawLine3D(inter.get((float)i * 0.01f), inter.get((float)i * 0.01f + 0.01f));
		nix::SetColor(White);
	}
	if (ready_for_scaling){
		geo->build(nix::vb_temp);
		nix::EnableLighting(true);
		mode_model->setMaterialCreation();
		nix::Draw3D(nix::vb_temp);
	}else if (pos.num > 2){
		vector pp = multi_view->mouse_win->project(pos[0]);
		pp.z = 0;
		if ((pp - multi_view->m).length_fuzzy() < CYLINDER_CLOSING_DISTANCE){
			ed->drawStr(pp.x, pp.y, _("Pfad schlie&sen"));
		}
	}
}

void ModeModelMeshCreateCylinderSnake::onClose()
{
	abort();
}
