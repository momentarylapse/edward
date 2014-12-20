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

ModeModelMeshCreateCylinderSnake::~ModeModelMeshCreateCylinderSnake()
{
}

void ModeModelMeshCreateCylinderSnake::onStart()
{
	dialog = HuiCreateResourceDialog("new_cylinder_dialog",ed);

	dialog->setInt("ncy_rings", HuiConfig.getInt("NewCylinderRings", 4));
	dialog->setInt("ncy_edges", HuiConfig.getInt("NewCylinderEdges", 8));
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->show();
	dialog->eventS("hui:close", &HuiFuncIgnore);

	ed->activate("");
}


void ModeModelMeshCreateCylinderSnake::onEnd()
{
	delete(dialog);
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateCylinderSnake::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (ready_for_scaling){
		int rings = dialog->getInt("ncy_rings");
		int edges = dialog->getInt("ncy_edges");
		HuiConfig.setInt("NewCylinderRings", rings);
		HuiConfig.setInt("NewCylinderEdges", edges);

		geo = new GeometryCylinder(pos, radius, rings * (pos.num - 1), edges, closed);
	}
}


void ModeModelMeshCreateCylinderSnake::onMouseMove()
{
	if (ready_for_scaling){
		vector p = multi_view->getCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->cam.zoom; // 10 px
		if (radius < min_rad)
			radius = min_rad;
		UpdateGeometry();
	}
}



void ModeModelMeshCreateCylinderSnake::onLeftButtonDown()
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
				UpdateGeometry();
				ed->forceRedraw();
				return;
			}

		}
		if (multi_view->hover.index >= 0)
			pos.add(data->vertex[multi_view->hover.index].pos);
		else
			pos.add(multi_view->getCursor3d());
	}
}



void ModeModelMeshCreateCylinderSnake::onKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		if (pos.num > 1){
			ready_for_scaling = true;
			onMouseMove();
			message = _("Zylinder: Radius");
			UpdateGeometry();
			ed->forceRedraw();
		}
	}
}





void ModeModelMeshCreateCylinderSnake::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos.num > 0){
		NixEnableLighting(false);
		// control polygon
		for (int i=0;i<pos.num;i++){
			vector pp = win->project(pos[i]);
			NixSetColor(Green);
			NixDrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
			NixSetColor(White);
			if (i > 0)
				NixDrawLine3D(pos[i - 1], pos[i]);
		}

		// spline curve
		Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
		foreach(vector &p, pos)
			inter.add(p);
		if (!ready_for_scaling)
			inter.add(multi_view->getCursor3d());
		inter.normalize();
		NixSetColor(Green);
		for (int i=0;i<100;i++)
			NixDrawLine3D(inter.get((float)i * 0.01f), inter.get((float)i * 0.01f + 0.01f));
		NixSetColor(White);
	}
	if (ready_for_scaling){
		geo->preview(VBTemp);
		NixEnableLighting(true);
		mode_model->setMaterialCreation();
		NixDraw3D(VBTemp);
	}else if (pos.num > 2){
		vector pp = multi_view->mouse_win->project(pos[0]);
		pp.z = 0;
		if ((pp - multi_view->m).length_fuzzy() < CYLINDER_CLOSING_DISTANCE){
			ed->drawStr(pp.x, pp.y, _("Pfad schlie&sen"));
		}
	}
}


