/*
 * ModeModelMeshCreateCylinder.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinder.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"

ModeModelMeshCreateCylinder::ModeModelMeshCreateCylinder(Mode *_parent) :
	ModeCreation("ModelMeshCreateCylinder", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Zylinder: Startpunkt");

	radius = 0;
}

ModeModelMeshCreateCylinder::~ModeModelMeshCreateCylinder()
{
}

void ModeModelMeshCreateCylinder::OnStart()
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


void ModeModelMeshCreateCylinder::OnEnd()
{
	delete(dialog);
}


void ModeModelMeshCreateCylinder::OnMouseMove()
{
	if (pos.num == 2){
		vector p = multi_view->GetCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->zoom; // 10 px
		if (radius < min_rad)
			radius = min_rad;
	}
}



void ModeModelMeshCreateCylinder::OnLeftButtonDown()
{
	if (pos.num == 2){

		int rings = dialog->GetInt("ncy_rings");
		int edges = dialog->GetInt("ncy_edges");
		bool closed = dialog->IsChecked("ncy_endings");
		HuiConfigWriteInt("NewCylinderRings", rings);
		HuiConfigWriteInt("NewCylinderEdges", edges);
		HuiConfigWriteBool("NewCylinderClosedEndings", closed);

		Array<float> r = radius;
		r += radius;

		ModelSurface *s = data->AddCylinder(pos, r, rings, edges, closed);
		data->SelectOnlySurface(s);

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos.add(data->Vertex[multi_view->Selected].pos);
		else
			pos.add(multi_view->GetCursor3d());

		if (pos.num > 1){
			//OnMouseMove();
			message = _("Zylinder: Radius");
			//ed->ForceRedraw();
		}else{
			message = _("Zylinder: Endpunkt");
		}
	}
}


void CreateCylinderBuffer(int buffer, const vector &pos, const vector &length, float radius)
{
	int num=16;
	vector u = length.ortho();
	u.normalize();
	vector r = length ^ u;
	r.normalize();
	for (int i=0;i<num;i++){
		float w1=pi*2*(float) i   /(float)num;
		float w2=pi*2*(float)(i+1)/(float)num;
		vector n1=u*(float)sin(w1)+r*(float)cos(w1);
		vector n2=u*(float)sin(w2)+r*(float)cos(w2);
		vector pa=pos       +n1*radius;
		vector pb=pos       +n2*radius;
		vector pc=pos+length+n1*radius;
		vector pd=pos+length+n2*radius;
		vector pe=pos+length;
		vector md=-length;
		NixVBAddTria(buffer,pa,md,0,0,pb,md,0,0,pos,md,0,0);
		NixVBAddTria(buffer,pa,n1,0,0,pc,n1,0,0,pd ,n2,0,0);
		NixVBAddTria(buffer,pa,n1,0,0,pd,n2,0,0,pb ,n2,0,0);
		NixVBAddTria(buffer,pd, length,0,0,pc, length,0,0,pe , length,0,0);
	}
}

void ModeModelMeshCreateCylinder::OnDrawWin(int win)
{
	if (pos.num > 0){
		NixEnableLighting(false);
		NixSetColor(Green);
		// control polygon
		for (int i=0;i<pos.num;i++){
			vector pp = multi_view->VecProject(pos[i], win);
			NixDrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}
		if (pos.num == 2)
			NixDrawLine3D(pos[0], pos[1]);
		else
			NixDrawLine3D(pos[0], multi_view->GetCursor3d());
		NixSetColor(White);
	}
	if (pos.num == 2){
		NixEnableLighting(true);
		mode_model->SetMaterialCreation();
		NixVBClear(VBTemp);
		CreateCylinderBuffer(VBTemp, pos[0], pos[1] - pos[0], radius);
		NixDraw3D(VBTemp);
	}
}


