/*
 * ModeModelMeshCreateCylinderSnake.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinderSnake.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/ModelGeometryCylinder.h"
#include "../../../../Edward.h"

// -> ModeModelMeshCreateCylinder.cpp
void CreateCylinderBuffer(int buffer, const vector &pos, const vector &length, float radius);


ModeModelMeshCreateCylinderSnake::ModeModelMeshCreateCylinderSnake(Mode *_parent) :
	ModeCreation("ModelMeshCreateCylinderSnake", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Zylinderschlange... Punkte + Shift Return");

	radius = 0;
	ready_for_scaling = false;
	geo = NULL;
}

ModeModelMeshCreateCylinderSnake::~ModeModelMeshCreateCylinderSnake()
{
}

void ModeModelMeshCreateCylinderSnake::OnStart()
{
	dialog = HuiCreateResourceDialog("new_cylinder_dialog",ed);

	dialog->SetInt("ncy_rings", HuiConfigReadInt("NewCylinderRings", 4));
	dialog->SetInt("ncy_edges", HuiConfigReadInt("NewCylinderEdges", 8));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}


void ModeModelMeshCreateCylinderSnake::OnEnd()
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
		int rings = dialog->GetInt("ncy_rings");
		int edges = dialog->GetInt("ncy_edges");
		HuiConfigWriteInt("NewCylinderRings", rings);
		HuiConfigWriteInt("NewCylinderEdges", edges);

		Array<float> r = radius;
		r += radius;
		geo = new ModelGeometryCylinder(pos, r, rings * (pos.num - 1), edges, true);
	}
}


void ModeModelMeshCreateCylinderSnake::OnMouseMove()
{
	if (ready_for_scaling){
		vector p = multi_view->GetCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->zoom; // 10 px
		if (radius < min_rad)
			radius = min_rad;
		UpdateGeometry();
	}
}



void ModeModelMeshCreateCylinderSnake::OnLeftButtonDown()
{
	if (ready_for_scaling){

		data->PasteGeometry(*geo);
		data->SelectOnlySurface(&data->Surface.back());

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos.add(data->Vertex[multi_view->Selected].pos);
		else
			pos.add(multi_view->GetCursor3d());
	}
}



void ModeModelMeshCreateCylinderSnake::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		if (pos.num > 1){
			ready_for_scaling = true;
			OnMouseMove();
			message = _("Zylinder: Radius");
			UpdateGeometry();
			ed->ForceRedraw();
		}
	}
}





void ModeModelMeshCreateCylinderSnake::OnDrawWin(int win)
{
	if (pos.num > 0){
		NixEnableLighting(false);
		// control polygon
		for (int i=0;i<pos.num;i++){
			vector pp = multi_view->VecProject(pos[i], win);
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
			inter.add(multi_view->GetCursor3d());
		NixSetColor(Green);
		for (int i=0;i<100;i++)
			NixDrawLine3D(inter.get((float)i * 0.01f), inter.get((float)i * 0.01f + 0.01f));
		NixSetColor(White);
	}
	if (ready_for_scaling){
		geo->Preview(VBTemp);
		NixEnableLighting(true);
		mode_model->SetMaterialCreation();
		NixDraw3D(VBTemp);
	}
}


