/*
 * ModeWorldEditTerrain.cpp
 *
 *  Created on: 31.08.2013
 *      Author: michi
 */

#include "ModeWorldTerrain.h"

#include "../../../Action/World/Terrain/ActionWorldTerrainBrushExtrude.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../MultiView/DrawingHelper.h"
#include "../../../lib/nix/nix.h"

const float BRUSH_PARTITION = 0.3f;

ModeWorldTerrain *mode_world_terrain = nullptr;





class TerrainDeformBrushPanel : public hui::Panel {
public:
	TerrainDeformBrushPanel(MultiView::MultiView *mv) {

		base_diameter = mv->cam.radius * 0.2f;
		base_depth = mv->cam.radius * 0.02f;

		from_resource("model-deformation-brush-dialog");

		event("diameter-slider", [=]{ on_diameter_slider(); });
		event("depth-slider", [=]{ on_depth_slider(); });

		add_string("brush-type", _("bump in/out"));
		set_float("diameter-slider", 0.5f);
		set_float("depth-slider", 0.5f);
		set_string("diameter", f2s(base_diameter, 2));
		set_string("depth", f2s(base_depth, 2));
		set_int("brush-type", 0);
	}

	void on_diameter_slider() {
		float x = get_float("");
		set_string("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
	}

	void on_depth_slider() {
		float x = get_float("");
		set_string("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
	}

	float base_diameter, base_depth;
};

ModeWorldTerrain::ModeWorldTerrain(ModeBase* _parent) :
	ModeCreation("WorldEditTerrain", _parent)
{
	brushing = false;
	message = _("paint onto terrain");
	base_depth = 1;
	base_diameter = 1;
	dialog = nullptr;
}

ModeWorldTerrain::~ModeWorldTerrain() {
}


Action *ModeWorldTerrain::get_action(const vector &pos) {
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

void ModeWorldTerrain::apply(const vector &pos) {
	Action *a = get_action(pos);
	data->execute(a);
	last_pos = pos;
}

void ModeWorldTerrain::on_start() {
	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);

	ed->toolbar[hui::TOOLBAR_LEFT]->reset();
	ed->toolbar[hui::TOOLBAR_LEFT]->enable(false);


	dialog = new TerrainDeformBrushPanel(multi_view);
	ed->set_side_panel(dialog);

}

void ModeWorldTerrain::on_end() {
	ed->set_side_panel(nullptr);
	multi_view->set_allow_action(true);
	multi_view->set_allow_select(true);
	if (brushing)
		data->end_action_group();
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("world-edit-toolbar"); // -> world
}

void ModeWorldTerrain::on_mouse_move() {
	if (!brushing)
		return;
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->get_float("diameter") / 2;
	vector dir = pos - last_pos;
	dir.normalize();
	float dl = radius * BRUSH_PARTITION;
	while ((pos - last_pos).length() > dl) {
		apply(last_pos + dl * dir);
	}
}

void ModeWorldTerrain::on_left_button_down() {
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	data->begin_action_group("TerrainBrush");
	vector pos = multi_view->hover.point;
	brushing = true;

	apply(pos);
}

void ModeWorldTerrain::on_left_button_up() {
	if (brushing)
		data->end_action_group();
	brushing = false;
}

void ModeWorldTerrain::on_command(const string& id) {
}

void ModeWorldTerrain::on_draw_win(MultiView::Window* win) {
	parent->on_draw_win(win);
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_WORLD_TERRAIN))
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->get_float("diameter") / 2;

	set_line_color(scheme.CREATION_LINE);
	set_wide_lines(scheme.LINE_WIDTH_HELPER);
	draw_circle(pos, vector::EY, radius);
}
