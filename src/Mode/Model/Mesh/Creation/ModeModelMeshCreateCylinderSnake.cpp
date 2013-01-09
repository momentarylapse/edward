/*
 * ModeModelMeshCreateCylinderSnake.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinderSnake.h"
#include "../../ModeModel.h"
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
}

ModeModelMeshCreateCylinderSnake::~ModeModelMeshCreateCylinderSnake()
{
}

void ModeModelMeshCreateCylinderSnake::OnStart()
{
	dialog = HuiCreateResourceDialog("new_cylinder_dialog",ed);

	dialog->SetInt("ncy_rings", HuiConfigReadInt("NewCylinderRings", 4));
	dialog->SetInt("ncy_edges", HuiConfigReadInt("NewCylinderEdges", 8));
	dialog->Check("ncy_endings", HuiConfigReadBool("NewCylinderClosedEndings", true));
	dialog->Check("ncy_texture_closed", HuiConfigReadBool("NewCylinderClosedTexture", true));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}


void ModeModelMeshCreateCylinderSnake::OnEnd()
{
	delete(dialog);
}


void ModeModelMeshCreateCylinderSnake::OnMouseMove()
{
	if (ready_for_scaling){
		vector p = multi_view->GetCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->zoom; // 10 px
		if (radius < min_rad)
			radius = min_rad;
	}
}



void ModeModelMeshCreateCylinderSnake::OnLeftButtonDown()
{
	if (ready_for_scaling){

		int rings = dialog->GetInt("ncy_rings");
		int edges = dialog->GetInt("ncy_edges");
		bool closed = dialog->IsChecked("ncy_endings");
		HuiConfigWriteInt("NewCylinderRings", rings);
		HuiConfigWriteInt("NewCylinderEdges", edges);
		HuiConfigWriteBool("NewCylinderClosedEndings", closed);

		Array<float> r = radius;
		r += radius;

		ModelSurface *s = data->AddCylinder(pos, r, rings * (pos.num - 1), edges, closed);
		data->SelectOnlySurface(s);

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos.add(data->Vertex[multi_view->Selected].pos);
		else
			pos.add(multi_view->GetCursor3d());
	//message = _("Zylinder: Endpunkt");
	}
}



void ModeModelMeshCreateCylinderSnake::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		if (pos.num > 1){
			ready_for_scaling = true;
			OnMouseMove();
			message = _("Zylinder: Radius");
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
		Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
		foreach(vector &p, pos)
			inter.add(p);
		int n = (pos.num - 1) * dialog->GetInt("ncy_rings");
		NixEnableLighting(true);
		mode_model->SetMaterialCreation();
		NixVBClear(VBTemp);
		for (int i=0;i<n;i++){
			float t0 = (float)i       / (float)n;
			float t1 = (float)(i + 1) / (float)n;
			CreateCylinderBuffer(VBTemp, inter.get(t0), inter.get(t1) - inter.get(t0), radius);
		}
		NixDraw3D(VBTemp);
	}
}


