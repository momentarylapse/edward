/*
 * ModeModelMeshBrush.cpp
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#include "ModeModelMeshBrush.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Brush/ActionModelBrushExtrude.h"
#include "../../../../Action/Model/Mesh/Brush/ActionModelBrushSmooth.h"


ModeModelMeshBrush::ModeModelMeshBrush(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshBrush", _parent)
{
	message = _("auf der Oberfl&ache malen");
	brushing = false;
}

ModeModelMeshBrush::~ModeModelMeshBrush()
{
}


Action *ModeModelMeshBrush::GetAction()
{
	vector pos = multi_view->hover.point;
	vector n = data->Surface[multi_view->hover.set].Polygon[multi_view->hover.index].TempNormal;
	float radius = dialog->GetFloat("diameter") / 2;
	float depth = dialog->GetFloat("depth");
	int type = dialog->GetInt("brush_type");
	if (ed->GetKey(KEY_CONTROL))
		depth = - depth;

	Action *a = NULL;
	if (type == 0)
		a = new ActionModelBrushExtrude(pos, n, radius, depth);
	else if (type == 1)
		a = new ActionModelBrushSmooth(pos, n, radius);
	return a;
}

void ModeModelMeshBrush::Apply()
{
	Action *a = GetAction();
	data->Execute(a);
}

void ModeModelMeshBrush::OnStart()
{
	multi_view->allow_mouse_actions = false;

	// Dialog
	dialog = new HuiFixedDialog(_("Pinsel"), 300, 155, ed, true);//HuiCreateResourceDialog("new_ball_dialog", ed);
	dialog->AddText(_("Dicke"), 5, 5, 80, 25, "");
	dialog->AddText(_("Tiefe"), 5, 35, 80, 25, "");
	dialog->AddSlider("", 90, 5, 115, 25, "diameter_slider");
	dialog->AddSlider("", 90, 35, 115, 25, "depth_slider");
	dialog->AddEdit("", 215, 5, 80, 25, "diameter");
	dialog->AddEdit("", 215, 35, 80, 25, "depth");
	dialog->AddListView("!nobar\\type", 5, 65, 290, 80, "brush_type");

	dialog->EventM("diameter_slider", this, &ModeModelMeshBrush::OnDiameterSlider);
	dialog->EventM("depth_slider", this, &ModeModelMeshBrush::OnDepthSlider);

	base_diameter = multi_view->cam.radius * 0.2f;
	base_depth = multi_view->cam.radius * 0.02f;

	dialog->AddString("brush_type", _("Ausbeulen/Eindellen"));
	dialog->AddString("brush_type", _("Gl&atten"));
	dialog->SetFloat("diameter_slider", 0.5f);
	dialog->SetFloat("depth_slider", 0.5f);
	dialog->SetString("diameter", f2s(base_diameter, 2));
	dialog->SetString("depth", f2s(base_depth, 2));
	dialog->SetInt("brush_type", 0);
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Show();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate("");
}

void ModeModelMeshBrush::OnDiameterSlider()
{
	float x = dialog->GetFloat("");
	dialog->SetString("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void ModeModelMeshBrush::OnDepthSlider()
{
	float x = dialog->GetFloat("");
	dialog->SetString("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
}

void ModeModelMeshBrush::OnEnd()
{
	delete(dialog);
	multi_view->allow_mouse_actions = true;
	if (brushing)
		data->EndActionGroup();
}

void ModeModelMeshBrush::OnLeftButtonDown()
{
	if (multi_view->hover.index < 0)
		return;
	data->BeginActionGroup("brush");
	vector pos = multi_view->hover.point;
	distance = 0;
	last_pos = pos;
	brushing = true;

	Apply();
}

void ModeModelMeshBrush::OnLeftButtonUp()
{
	if (brushing)
		data->EndActionGroup();
	brushing = false;
}

void ModeModelMeshBrush::OnMouseMove()
{
	if (!brushing)
		return;
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->GetFloat("diameter") / 2;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius * 0.7f){
		distance = 0;
		Apply();
	}
}

void ModeModelMeshBrush::OnDrawWin(MultiViewWindow* win)
{
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	vector n = data->Surface[multi_view->hover.set].Polygon[multi_view->hover.index].TempNormal;
	float radius = dialog->GetFloat("diameter") / 2;

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
}
