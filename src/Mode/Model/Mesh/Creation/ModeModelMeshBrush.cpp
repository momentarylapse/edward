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
#include "../../../../Action/Model/Mesh/Brush/ActionModelBrushComplexify.h"


ModeModelMeshBrush::ModeModelMeshBrush(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshBrush", _parent)
{
	message = _("auf der Oberfl&ache malen");

	brushing = false;
	distance = 1;
	base_diameter = 1;
	base_depth = 1;
}


Action *ModeModelMeshBrush::getAction()
{
	vector pos = multi_view->hover.point;
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->getFloat("diameter") / 2;
	float depth = dialog->getFloat("depth");
	int type = dialog->getInt("brush_type");
	if (ed->getKey(hui::KEY_CONTROL))
		depth = - depth;

	Action *a = NULL;
	if (type == 0)
		a = new ActionModelBrushExtrude(pos, n, radius, depth);
	else if (type == 1)
		a = new ActionModelBrushSmooth(pos, n, radius);
	else if (type == 2)
		a = new ActionModelBrushComplexify(pos, n, radius, depth);
	return a;
}

void ModeModelMeshBrush::apply()
{
	Action *a = getAction();
	data->execute(a);
}

void ModeModelMeshBrush::onStart()
{
	mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_polygon);
	mode_model->allowSelectionModes(false);

	multi_view->setAllowAction(false);
	multi_view->setAllowSelect(false);

	// Dialog
	dialog = new hui::Dialog(_("Pinsel"), 300, 155, ed, true);//HuiCreateResourceDialog("new_ball_dialog", ed);
	dialog->addGrid("", 0, 0, "grid");

	dialog->setTarget("grid");
	dialog->addGrid("", 0, 0, "grid1");
	dialog->addListView("!nobar\\type", 0, 1, "brush_type");

	dialog->setTarget("grid1");
	dialog->addLabel(_("Dicke"), 0, 0, "");
	dialog->addLabel(_("Tiefe"), 0, 1, "");
	dialog->addSlider("", 1, 0, "diameter_slider");
	dialog->addSlider("", 1, 1, "depth_slider");
	dialog->addEdit("", 2, 0, "diameter");
	dialog->addEdit("", 2, 1, "depth");

	dialog->event("diameter_slider", std::bind(&ModeModelMeshBrush::onDiameterSlider, this));
	dialog->event("depth_slider", std::bind(&ModeModelMeshBrush::onDepthSlider, this));

	base_diameter = multi_view->cam.radius * 0.2f;
	base_depth = multi_view->cam.radius * 0.02f;

	dialog->addString("brush_type", _("Ausbeulen/Eindellen"));
	dialog->addString("brush_type", _("Gl&atten"));
	dialog->addString("brush_type", _("Komplexifizieren"));
	dialog->setFloat("diameter_slider", 0.5f);
	dialog->setFloat("depth_slider", 0.5f);
	dialog->setString("diameter", f2s(base_diameter, 2));
	dialog->setString("depth", f2s(base_depth, 2));
	dialog->setInt("brush_type", 0);
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshBrush::onClose, this));

	ed->activate("");
}

void ModeModelMeshBrush::onDiameterSlider()
{
	float x = dialog->getFloat("");
	dialog->setString("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void ModeModelMeshBrush::onDepthSlider()
{
	float x = dialog->getFloat("");
	dialog->setString("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
}

void ModeModelMeshBrush::onEnd()
{
	delete(dialog);
	if (brushing)
		data->endActionGroup();
	mode_model->allowSelectionModes(true);
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

	apply();
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
		apply();
	}
}

void ModeModelMeshBrush::onDrawWin(MultiView::Window* win)
{
	parent->onDrawWin(win);

	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->getFloat("diameter") / 2;

	nix::SetColor(Green);
	nix::EnableLighting(false);
	vector e1 = n.ortho();
	vector e2 = n ^ e1;
	e1 *= radius;
	e2 *= radius;
	for (int i=0;i<32;i++){
		float w1 = i * 2 * pi / 32;
		float w2 = (i + 1) * 2 * pi / 32;
		nix::DrawLine3D(pos + sin(w1) * e1 + cos(w1) * e2, pos + sin(w2) * e1 + cos(w2) * e2);
	}
}

void ModeModelMeshBrush::onClose()
{
	abort();
}
