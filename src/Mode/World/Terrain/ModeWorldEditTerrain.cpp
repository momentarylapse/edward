/*
 * ModeWorldEditTerrain.cpp
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#include "ModeWorldEditTerrain.h"
#include "../../../Action/World/Terrain/ActionWorldTerrainBrushExtrude.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../lib/nix/nix.h"

const float BRUSH_PARTITION = 0.3f;

ModeWorldEditTerrain::ModeWorldEditTerrain(ModeBase* _parent) :
	ModeCreation("WorldEditTerrain", _parent)
{
	brushing = false;
	message = _("auf das Terrain malen");
	base_depth = 1;
	base_diameter = 1;
}

ModeWorldEditTerrain::~ModeWorldEditTerrain()
{
}


Action *ModeWorldEditTerrain::getAction(const vector &pos)
{
	float radius = dialog->getFloat("diameter") / 2;
	float depth = dialog->getFloat("depth") * BRUSH_PARTITION;
	int type = dialog->getInt("brush_type");
	if (ed->getKey(hui::KEY_CONTROL))
		depth = - depth;

	Action *a = NULL;
	if (type == 0)
		a = new ActionWorldTerrainBrushExtrude(multi_view->hover.index, pos, radius, depth);
	return a;
}

void ModeWorldEditTerrain::apply(const vector &pos)
{
	Action *a = getAction(pos);
	data->execute(a);
	last_pos = pos;
}

void ModeWorldEditTerrain::onStart()
{
	multi_view->setAllowAction(false);
	multi_view->setAllowSelect(false);

	// Dialog
	dialog = new hui::Dialog(_("Pinsel"), 300, 155, ed, true);//HuiCreateResourceDialog("new_ball_dialog", ed);
	dialog->addGrid("", 0, 0, "grid1");
	dialog->setTarget("grid1");
	dialog->addGrid("", 0, 0, "grid2");
	dialog->addListView("!nobar\\type", 0, 1, "brush_type");
	dialog->setTarget("grid2");
	dialog->addLabel(_("Dicke"), 0, 0, "");
	dialog->addLabel(_("Tiefe"), 0, 1, "");
	dialog->addSlider("!expandx", 1, 0, "diameter_slider");
	dialog->addSlider("", 1, 1, "depth_slider");
	dialog->addEdit("!width=60", 2, 0, "diameter");
	dialog->addEdit("", 2, 1, "depth");

	dialog->event("diameter_slider", std::bind(&ModeWorldEditTerrain::onDiameterSlider, this));
	dialog->event("depth_slider", std::bind(&ModeWorldEditTerrain::onDepthSlider, this));

	base_diameter = multi_view->cam.radius * 0.2f;
	base_depth = multi_view->cam.radius * 0.02f;

	dialog->addString("brush_type", _("Ausbeulen/Eindellen"));
//	dialog->addString("brush_type", _("Gl&atten"));
	dialog->setFloat("diameter_slider", 0.5f);
	dialog->setFloat("depth_slider", 0.5f);
	dialog->setString("diameter", f2s(base_diameter, 2));
	dialog->setString("depth", f2s(base_depth, 2));
	dialog->setInt("brush_type", 0);
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", hui::FuncIgnore);

	ed->activate("");
}

void ModeWorldEditTerrain::onEnd()
{
	delete(dialog);
	multi_view->setAllowAction(true);
	multi_view->setAllowSelect(true);
	if (brushing)
		data->endActionGroup();
}

void ModeWorldEditTerrain::onDiameterSlider()
{
	float x = dialog->getFloat("");
	dialog->setString("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void ModeWorldEditTerrain::onDepthSlider()
{
	float x = dialog->getFloat("");
	dialog->setString("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
}

void ModeWorldEditTerrain::onMouseMove()
{
	if (!brushing)
		return;
	if ((multi_view->hover.index < 0) || (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->getFloat("diameter") / 2;
	vector dir = pos - last_pos;
	dir.normalize();
	float dl = radius * BRUSH_PARTITION;
	while ((pos - last_pos).length() > dl){
		apply(last_pos + dl * dir);
	}
}

void ModeWorldEditTerrain::onLeftButtonDown()
{
	if ((multi_view->hover.index < 0) || (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	data->beginActionGroup("TerrainBrush");
	vector pos = multi_view->hover.point;
	brushing = true;

	apply(pos);
}

void ModeWorldEditTerrain::onLeftButtonUp()
{
	if (brushing)
		data->endActionGroup();
	brushing = false;
}

void ModeWorldEditTerrain::onCommand(const string& id)
{
}

void ModeWorldEditTerrain::onDrawWin(MultiView::Window* win)
{
	parent->onDrawWin(win);
	if ((multi_view->hover.index < 0) || (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->getFloat("diameter") / 2;

	nix::SetColor(Green);
	MultiView::set_wide_lines(2);
	vector e1 = e_x;
	vector e2 = e_z;
	e1 *= radius;
	e2 *= radius;
	for (int i=0;i<32;i++){
		float w1 = i * 2 * pi / 32;
		float w2 = (i + 1) * 2 * pi / 32;
		nix::DrawLine3D(pos + sin(w1) * e1 + cos(w1) * e2, pos + sin(w2) * e1 + cos(w2) * e2);
	}
}
