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
	float radius = dialog->get_float("diameter") / 2;
	float depth = dialog->get_float("depth") * BRUSH_PARTITION;
	int type = dialog->get_int("brush_type");
	if (ed->get_key(hui::KEY_CONTROL))
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

void ModeWorldEditTerrain::on_start()
{
	multi_view->setAllowAction(false);
	multi_view->setAllowSelect(false);

	// Dialog
	dialog = new hui::Dialog(_("Pinsel"), 300, 155, ed, true);//HuiCreateResourceDialog("new_ball_dialog", ed);
	dialog->add_grid("", 0, 0, "grid1");
	dialog->set_target("grid1");
	dialog->add_grid("", 0, 0, "grid2");
	dialog->add_list_view("!nobar\\type", 0, 1, "brush_type");
	dialog->set_target("grid2");
	dialog->add_label(_("Dicke"), 0, 0, "");
	dialog->add_label(_("Tiefe"), 0, 1, "");
	dialog->add_slider("!expandx", 1, 0, "diameter_slider");
	dialog->add_slider("", 1, 1, "depth_slider");
	dialog->add_edit("!width=60", 2, 0, "diameter");
	dialog->add_edit("", 2, 1, "depth");

	dialog->event("diameter_slider", std::bind(&ModeWorldEditTerrain::onDiameterSlider, this));
	dialog->event("depth_slider", std::bind(&ModeWorldEditTerrain::onDepthSlider, this));

	base_diameter = multi_view->cam.radius * 0.2f;
	base_depth = multi_view->cam.radius * 0.02f;

	dialog->add_string("brush_type", _("Ausbeulen/Eindellen"));
//	dialog->addString("brush_type", _("Gl&atten"));
	dialog->set_float("diameter_slider", 0.5f);
	dialog->set_float("depth_slider", 0.5f);
	dialog->set_string("diameter", f2s(base_diameter, 2));
	dialog->set_string("depth", f2s(base_depth, 2));
	dialog->set_int("brush_type", 0);
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", [=]{ });

	ed->activate("");
}

void ModeWorldEditTerrain::on_end()
{
	delete(dialog);
	multi_view->setAllowAction(true);
	multi_view->setAllowSelect(true);
	if (brushing)
		data->end_action_group();
}

void ModeWorldEditTerrain::onDiameterSlider()
{
	float x = dialog->get_float("");
	dialog->set_string("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void ModeWorldEditTerrain::onDepthSlider()
{
	float x = dialog->get_float("");
	dialog->set_string("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
}

void ModeWorldEditTerrain::on_mouse_move()
{
	if (!brushing)
		return;
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->get_float("diameter") / 2;
	vector dir = pos - last_pos;
	dir.normalize();
	float dl = radius * BRUSH_PARTITION;
	while ((pos - last_pos).length() > dl){
		apply(last_pos + dl * dir);
	}
}

void ModeWorldEditTerrain::on_left_button_down()
{
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	data->begin_action_group("TerrainBrush");
	vector pos = multi_view->hover.point;
	brushing = true;

	apply(pos);
}

void ModeWorldEditTerrain::on_left_button_up()
{
	if (brushing)
		data->end_action_group();
	brushing = false;
}

void ModeWorldEditTerrain::on_command(const string& id)
{
}

void ModeWorldEditTerrain::on_draw_win(MultiView::Window* win)
{
	parent->on_draw_win(win);
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->get_float("diameter") / 2;

	nix::SetColor(multi_view->ColorCreationLine);
	MultiView::set_wide_lines(2);
	vector e1 = vector::EX;
	vector e2 = vector::EZ;
	e1 *= radius;
	e2 *= radius;
	for (int i=0;i<32;i++){
		float w1 = i * 2 * pi / 32;
		float w2 = (i + 1) * 2 * pi / 32;
		nix::DrawLine3D(pos + sin(w1) * e1 + cos(w1) * e2, pos + sin(w2) * e1 + cos(w2) * e2);
	}
}
