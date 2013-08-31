/*
 * ModeModelMeshBrushExtrudeVertices.cpp
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#include "ModeModelMeshBrushExtrudeVertices.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"


ModeModelMeshBrushExtrudeVertices::ModeModelMeshBrushExtrudeVertices(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshBrushExtrudeVertices", _parent)
{
	message = _("Punkte auf der Oberfl&ache anklicken");
	brushing = false;
}

ModeModelMeshBrushExtrudeVertices::~ModeModelMeshBrushExtrudeVertices()
{
}

void ModeModelMeshBrushExtrudeVertices::OnStart()
{
	multi_view->allow_mouse_actions = false;

	// Dialog
	dialog = new HuiFixedDialog(_("Pinsel"), 300, 100, ed, true);//HuiCreateResourceDialog("new_ball_dialog", ed);
	dialog->AddText(_("Dicke"), 5, 5, 80, 25, "");
	dialog->AddText(_("Tiefe"), 5, 35, 80, 25, "");
	dialog->AddEdit("", 90, 5, 80, 25, "radius");
	dialog->AddEdit("", 90, 35, 80, 25, "depth");

	dialog->SetString("radius", f2s(multi_view->cam.radius * 0.1f, 2));
	dialog->SetString("depth", f2s(multi_view->cam.radius * 0.02f, 2));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Show();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}

void ModeModelMeshBrushExtrudeVertices::OnEnd()
{
	delete(dialog);
	multi_view->allow_mouse_actions = true;
	if (brushing)
		data->EndActionGroup();
}

void ModeModelMeshBrushExtrudeVertices::OnLeftButtonDown()
{
	if (multi_view->MouseOver < 0)
		return;
	data->BeginActionGroup("brush");
	vector pos = multi_view->MouseOverTP;
	vector n = data->Surface[multi_view->MouseOverSet].Polygon[multi_view->MouseOver].TempNormal;
	float radius = dialog->GetFloat("radius");
	float depth = dialog->GetFloat("depth");
	distance = 0;
	last_pos = pos;
	brushing = true;

	data->BrushExtrudeVertices(pos, n, radius, depth);
}

void ModeModelMeshBrushExtrudeVertices::OnLeftButtonUp()
{
	if (brushing)
		data->EndActionGroup();
	brushing = false;
}

void ModeModelMeshBrushExtrudeVertices::OnMouseMove()
{
	if (!HuiGetEvent()->lbut)
		return;
	if (multi_view->MouseOver < 0)
		return;
	float radius = dialog->GetFloat("radius");
	float depth = dialog->GetFloat("depth");
	if (ed->GetKey(KEY_CONTROL))
		depth = - depth;
	vector pos = multi_view->MouseOverTP;
	vector n = data->Surface[multi_view->MouseOverSet].Polygon[multi_view->MouseOver].TempNormal;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius * 0.7f){
		distance = 0;
		data->BrushExtrudeVertices(pos, n, radius, depth);
	}
}

void ModeModelMeshBrushExtrudeVertices::OnDrawWin(MultiViewWindow* win)
{
	if (multi_view->MouseOver < 0)
		return;
	vector pos = multi_view->MouseOverTP;
	vector n = data->Surface[multi_view->MouseOverSet].Polygon[multi_view->MouseOver].TempNormal;
	float radius = dialog->GetFloat("radius");
	float depth = dialog->GetFloat("depth");
	if (ed->GetKey(KEY_CONTROL))
		depth = - depth;

	NixSetColor(Green);
	NixEnableLighting(false);
	vector e1 = n.ortho();
	vector e2 = n ^ e1;
	e1 *= radius;
	e2 *= radius;
	for (int i=0;i<32;i++){
		float w1 = i * 2 * pi / 32;
		float w2 = (i + 1) * 2 * pi / 32;
		NixDrawLine3D(pos + sin(w1) * e1 + cos(w1) * e2, pos + sin(w2) * e1 + cos(w2) * e2);
	}
	NixDrawLine3D(pos, pos + n * depth);
}
