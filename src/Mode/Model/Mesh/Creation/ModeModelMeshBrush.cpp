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
#include "../../../../MultiView/MultiView.h"
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
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->getFloat("diameter") / 2;
	float depth = dialog->getFloat("depth");
	int type = dialog->getInt("brush_type");
	if (ed->getKey(KEY_CONTROL))
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
	data->execute(a);
}

void ModeModelMeshBrush::onStart()
{
	multi_view->SetAllowAction(false);
	multi_view->SetAllowRect(false);

	// Dialog
	dialog = new HuiFixedDialog(_("Pinsel"), 300, 155, ed, true);//HuiCreateResourceDialog("new_ball_dialog", ed);
	dialog->addText(_("Dicke"), 5, 5, 80, 25, "");
	dialog->addText(_("Tiefe"), 5, 35, 80, 25, "");
	dialog->addSlider("", 90, 5, 115, 25, "diameter_slider");
	dialog->addSlider("", 90, 35, 115, 25, "depth_slider");
	dialog->addEdit("", 215, 5, 80, 25, "diameter");
	dialog->addEdit("", 215, 35, 80, 25, "depth");
	dialog->addListView("!nobar\\type", 5, 65, 290, 80, "brush_type");

	dialog->event("diameter_slider", this, &ModeModelMeshBrush::OnDiameterSlider);
	dialog->event("depth_slider", this, &ModeModelMeshBrush::OnDepthSlider);

	base_diameter = multi_view->cam.radius * 0.2f;
	base_depth = multi_view->cam.radius * 0.02f;

	dialog->addString("brush_type", _("Ausbeulen/Eindellen"));
	dialog->addString("brush_type", _("Gl&atten"));
	dialog->setFloat("diameter_slider", 0.5f);
	dialog->setFloat("depth_slider", 0.5f);
	dialog->setString("diameter", f2s(base_diameter, 2));
	dialog->setString("depth", f2s(base_depth, 2));
	dialog->setInt("brush_type", 0);
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->show();
	dialog->eventS("hui:close", &HuiFuncIgnore);

	ed->activate("");
}

void ModeModelMeshBrush::OnDiameterSlider()
{
	float x = dialog->getFloat("");
	dialog->setString("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void ModeModelMeshBrush::OnDepthSlider()
{
	float x = dialog->getFloat("");
	dialog->setString("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
}

void ModeModelMeshBrush::onEnd()
{
	delete(dialog);
	multi_view->SetAllowAction(true);
	multi_view->SetAllowRect(true);
	if (brushing)
		data->endActionGroup();
}

void ModeModelMeshBrush::onLeftButtonDown()
{
	if (multi_view->hover.index < 0)
		return;
	data->beginActionGroup("brush");
	vector pos = multi_view->hover.point;
	distance = 0;
	last_pos = pos;
	brushing = true;

	Apply();
}

void ModeModelMeshBrush::onLeftButtonUp()
{
	if (brushing)
		data->endActionGroup();
	brushing = false;
}

void ModeModelMeshBrush::onMouseMove()
{
	if (!brushing)
		return;
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->getFloat("diameter") / 2;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius * 0.7f){
		distance = 0;
		Apply();
	}
}

void ModeModelMeshBrush::onDrawWin(MultiView::Window* win)
{
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->getFloat("diameter") / 2;

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
