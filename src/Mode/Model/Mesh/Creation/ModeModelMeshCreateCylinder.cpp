/*
 * ModeModelMeshCreateCylinder.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinder.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/ActionModelAddCylinder.h"

ModeModelMeshCreateCylinder::ModeModelMeshCreateCylinder(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshCreateCylinder";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("zylinder... Punkte + Shift Return");

	dialog = HuiCreateResourceDialog("new_cylinder_dialog",ed);

	dialog->SetInt("ncy_rings", HuiConfigReadInt("NewCylinderRings", 4));
	dialog->SetInt("ncy_edges", HuiConfigReadInt("NewCylinderEdges", 8));
	dialog->Check("ncy_endings", HuiConfigReadBool("NewCylinderClosedEndings", true));
	dialog->Check("ncy_texture_closed", HuiConfigReadBool("NewCylinderClosedTexture", true));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
	radius = 0;
	ready_for_scaling = false;
}

ModeModelMeshCreateCylinder::~ModeModelMeshCreateCylinder()
{
	delete(dialog);
}


void ModeModelMeshCreateCylinder::OnMouseMove()
{
	if (ready_for_scaling){
		vector p = multi_view->GetCursor3d();
		radius = VecLength(p - pos.back());
		float min_rad = 10 / multi_view->zoom; // 10 px
		if (radius < min_rad)
			radius = min_rad;
	}
}



void ModeModelMeshCreateCylinder::OnLeftButtonDown()
{
	if (ready_for_scaling){

		int rings = dialog->GetInt("ncy_rings");
		int edges = dialog->GetInt("ncy_edges");
		bool closed = dialog->IsChecked("ncy_endings");
		HuiConfigWriteInt("NewCylinderRings", rings);
		HuiConfigWriteInt("NewCylinderEdges", edges);
		HuiConfigWriteBool("NewCylinderClosedEndings", closed);

		data->Execute(new ActionModelAddCylinder(data, pos, radius, rings, edges, closed));

		ed->SetCreationMode(NULL);
	}else{
		if (multi_view->Selected >= 0)
			pos.add(data->Vertex[multi_view->Selected].pos);
		else
			pos.add(multi_view->GetCursor3d());
		radius = VecLength(pos.back() - pos[0]) / 8;
	//message = _("Zylinder: Endpunkt");
	}
}



void ModeModelMeshCreateCylinder::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		if (pos.num > 1){
			ready_for_scaling = true;
			message = _("Zylinder: Radius");
			ed->ForceRedraw();
		}
	}
}




void CreateCylinderBuffer(int buffer, const vector &pos, const vector &length, float radius)
{
	int num=16;
	vector u = VecOrtho(length);
	VecNormalize(u);
	vector r = length ^ u;
	VecNormalize(r);
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

void ModeModelMeshCreateCylinder::PostDrawWin(int win, irect dest)
{
	if (ready_for_scaling){
		Interpolator inter(Interpolator::TYPE_CUBIC_SPLINE_NOTANG);
		foreach(pos, p)
			inter.Add(p);
		int n = (pos.num - 1) * dialog->GetInt("ncy_rings");
		mode_model->SetMaterialCreation();
		NixVBClear(VBTemp);
		for (int i=0;i<n;i++){
			float t0 = (float)i       / (float)n;
			float t1 = (float)(i + 1) / (float)n;
			CreateCylinderBuffer(VBTemp, inter.Get(t0), inter.Get(t1) - inter.Get(t0), radius);
		}
		NixDraw3D(-1, VBTemp, m_id);
	}else if (pos.num > 0){
		Interpolator inter(Interpolator::TYPE_CUBIC_SPLINE_NOTANG);
		foreach(pos, p)
			inter.Add(p);
		inter.Add(multi_view->GetCursor3d());
		for (int i=0;i<100;i++)
			NixDrawLine3D(inter.Get((float)i * 0.01f), inter.Get((float)i * 0.01f + 0.01f), Green);
	}
}


