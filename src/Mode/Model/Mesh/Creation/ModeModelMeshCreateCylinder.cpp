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

	message = _("zylinder...");

	dialog = HuiCreateResourceDialog("new_cylinder_dialog",ed);

	dialog->SetInt("ncy_rings", HuiConfigReadInt("NewCylinderRings", 4));
	dialog->SetInt("ncy_edges", HuiConfigReadInt("NewCylinderEdges", 8));
	dialog->Check("ncy_endings", HuiConfigReadBool("NewCylinderClosedEndings", true));
	dialog->Check("ncy_texture_closed", HuiConfigReadBool("NewCylinderClosedTexture", true));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
	pos_chosen = false;
	pos2_chosen = false;
	radius = 0;
}

ModeModelMeshCreateCylinder::~ModeModelMeshCreateCylinder()
{
	delete(dialog);
}

void ModeModelMeshCreateCylinder::OnMiddleButtonDown()
{
}



void ModeModelMeshCreateCylinder::PostDraw()
{
}



void ModeModelMeshCreateCylinder::OnMiddleButtonUp()
{
}



void ModeModelMeshCreateCylinder::End()
{
}



void ModeModelMeshCreateCylinder::OnKeyDown()
{
}



void ModeModelMeshCreateCylinder::OnCommand(const string & id)
{
}



void ModeModelMeshCreateCylinder::OnRightButtonDown()
{
}



void ModeModelMeshCreateCylinder::OnMouseMove()
{
	if (pos_chosen){
		if (pos2_chosen){
			vector p = multi_view->GetCursor3d();
			radius = VecLength(p - pos2);
		}else{
			pos2 = multi_view->GetCursor3d();
		}
	}
}



void ModeModelMeshCreateCylinder::Start()
{
}



void ModeModelMeshCreateCylinder::OnRightButtonUp()
{
}



void ModeModelMeshCreateCylinder::OnLeftButtonDown()
{
	if (pos_chosen){
		if (pos2_chosen){

			int rings = dialog->GetInt("ncy_rings");
			int edges = dialog->GetInt("ncy_edges");
			bool closed = dialog->IsChecked("ncy_endings");
			HuiConfigWriteInt("NewCylinderRings", rings);
			HuiConfigWriteInt("NewCylinderEdges", edges);
			HuiConfigWriteBool("NewCylinderClosedEndings", closed);

			data->Execute(new ActionModelAddCylinder(pos, pos2 - pos, radius, rings, edges, closed, data->Vertex.num));

			ed->SetCreationMode(NULL);
		}else{
			if (multi_view->Selected >= 0)
				pos2 = data->Vertex[multi_view->Selected].pos;
			else
				pos2 = multi_view->GetCursor3d();
			message = _("Zylinder: Radius");
			pos2_chosen = true;
		}
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Zylinder: Endpunkt");
		pos_chosen = true;
		pos2 = pos;
	}
}



void ModeModelMeshCreateCylinder::OnLeftButtonUp()
{
}




void CreateCylinderBuffer(int buffer, const vector &pos, const vector &length, float radius)
{
	NixVBClear(buffer);
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
	if (pos_chosen){
		if (pos2_chosen){
			mode_model->SetMaterialCreation();
			CreateCylinderBuffer(VBTemp, pos, pos2 - pos, radius);
			NixDraw3D(-1, VBTemp, m_id);
		}else{
			NixDrawLine3D(pos, pos2, Green);
		}
	}
}



void ModeModelMeshCreateCylinder::OnKeyUp()
{
}


