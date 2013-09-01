/*
 * ModeWorldEditTerrain.cpp
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#include "ModeWorldEditTerrain.h"
#include "../../../Action/World/Terrain/ActionWorldTerrainBrushExtrude.h"
#include "../../../Edward.h"

ModeWorldEditTerrain::ModeWorldEditTerrain(ModeBase* _parent, int _index) :
	ModeCreation("WorldEditTerrain", _parent)
{
	index = _index;
	brushing = false;
	terrain = &data->Terrains[index];
	message = _("auf das Terrain malen");
}

ModeWorldEditTerrain::~ModeWorldEditTerrain()
{
}


Action *ModeWorldEditTerrain::GetAction()
{
	vector pos = multi_view->MouseOverTP;
	float radius = dialog->GetFloat("diameter") / 2;
	float depth = dialog->GetFloat("depth");
	int type = dialog->GetInt("brush_type");
	if (ed->GetKey(KEY_CONTROL))
		depth = - depth;

	Action *a = NULL;
	if (type == 0)
		a = new ActionWorldTerrainBrushExtrude(index, pos, radius, depth);
	return a;
}

void ModeWorldEditTerrain::Apply()
{
	Action *a = GetAction();
	data->Execute(a);
}

void ModeWorldEditTerrain::OnStart()
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

	dialog->EventM("diameter_slider", this, &ModeWorldEditTerrain::OnDiameterSlider);
	dialog->EventM("depth_slider", this, &ModeWorldEditTerrain::OnDepthSlider);

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

	ed->Activate();
}

void ModeWorldEditTerrain::OnEnd()
{
	delete(dialog);
	multi_view->allow_mouse_actions = true;
	if (brushing)
		data->EndActionGroup();
}

void ModeWorldEditTerrain::OnDiameterSlider()
{
	float x = dialog->GetFloat("");
	dialog->SetString("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void ModeWorldEditTerrain::OnDepthSlider()
{
	float x = dialog->GetFloat("");
	dialog->SetString("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
}

void ModeWorldEditTerrain::OnMouseMove()
{
	if (!HuiGetEvent()->lbut)
		return;
	if (multi_view->MouseOver < 0)
		return;
	vector pos = multi_view->MouseOverTP;
	float radius = dialog->GetFloat("diameter") / 2;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius * 0.7f){
		distance = 0;
		Apply();
	}
}

void ModeWorldEditTerrain::OnLeftButtonDown()
{
	if (multi_view->MouseOver < 0)
		return;
	data->BeginActionGroup("brush");
	vector pos = multi_view->MouseOverTP;
	distance = 0;
	last_pos = pos;
	brushing = true;

	Apply();
}

void ModeWorldEditTerrain::OnLeftButtonUp()
{
	if (brushing)
		data->EndActionGroup();
	brushing = false;
}

void ModeWorldEditTerrain::OnCommand(const string& id)
{
}

void ModeWorldEditTerrain::OnDrawWin(MultiViewWindow* win)
{
	if (multi_view->MouseOver < 0)
		return;
	vector pos = multi_view->MouseOverTP;
	float radius = dialog->GetFloat("diameter") / 2;

	NixSetColor(Green);
	NixEnableLighting(false);
	vector e1 = e_x;
	vector e2 = e_z;
	e1 *= radius;
	e2 *= radius;
	for (int i=0;i<32;i++){
		float w1 = i * 2 * pi / 32;
		float w2 = (i + 1) * 2 * pi / 32;
		NixDrawLine3D(pos + sin(w1) * e1 + cos(w1) * e2, pos + sin(w2) * e1 + cos(w2) * e2);
	}
}
