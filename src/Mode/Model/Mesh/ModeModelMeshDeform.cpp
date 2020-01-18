/*
 * ModeModelMeshDeform.cpp
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#include "ModeModelMeshDeform.h"
#include "ModeModelMesh.h"
#include "../ModeModel.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/DrawingHelper.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../Edward.h"
#include "../../../lib/nix/nix.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushExtrude.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushSmooth.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushComplexify.h"

ModeModelMeshDeform *mode_model_mesh_deform = NULL;


class DeformBrushPanel : public hui::Panel {
public:
	DeformBrushPanel(MultiView::MultiView *mv) {

		base_diameter = mv->cam.radius * 0.2f;
		base_depth = mv->cam.radius * 0.02f;

		from_resource("model-deformation-brush-dialog");

		event("diameter-slider", [=]{ on_diameter_slider(); });
		event("depth-slider", [=]{ on_depth_slider(); });

		add_string("brush-type", _("Ausbeulen/eindellen"));
		add_string("brush-type", _("Gl&atten"));
		add_string("brush-type", _("Komplexifizieren"));
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

ModeModelMeshDeform::ModeModelMeshDeform(ModeBase *_parent) :
			Mode<DataModel>("ModelMeshDeform", _parent, ed->multi_view_3d, "menu_model"),
		Observable("ModelMeshDeform") {
	dialog = nullptr;
	brushing = false;
	distance = 1;
}

ModeModelMeshDeform::~ModeModelMeshDeform() {
}

void ModeModelMeshDeform::on_start() {

	dialog = new DeformBrushPanel(multi_view);
	ed->set_side_panel(dialog);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);
	multi_view->set_allow_action(false);

	// enter
	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	mode_model->allow_selection_modes(false);
	mode_model_mesh->set_allow_draw_hover(false);



	multi_view->set_allow_select(false);
}

void ModeModelMeshDeform::on_end() {
	if (brushing)
		data->end_action_group();

	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->set_allow_action(true);
	mode_model->allow_selection_modes(true);
	mode_model_mesh->set_allow_draw_hover(true);
	ed->set_side_panel(nullptr);
}

void ModeModelMeshDeform::on_set_multi_view() {
	parent->on_set_multi_view();
}

void ModeModelMeshDeform::on_draw() {
	parent->on_draw();
}

void ModeModelMeshDeform::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->get_float("diameter") / 2;

	nix::SetColor(scheme.CREATION_LINE);
	set_wide_lines(scheme.LINE_WIDTH_MEDIUM);
	draw_circle(pos, n, radius);
}

void ModeModelMeshDeform::on_selection_change() {
	parent->on_selection_change();
}


Action *ModeModelMeshDeform::get_action() {
	vector pos = multi_view->hover.point;
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->get_float("diameter") / 2;
	float depth = dialog->get_float("depth");
	int type = dialog->get_int("brush-type");
	if (ed->get_key(hui::KEY_CONTROL))
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

void ModeModelMeshDeform::on_left_button_down() {
	if (multi_view->hover.index < 0)
		return;
	data->begin_action_group("brush");
	vector pos = multi_view->hover.point;
	distance = 0;
	last_pos = pos;
	brushing = true;

	apply();
}

void ModeModelMeshDeform::on_left_button_up() {
	if (brushing)
		data->end_action_group();
	brushing = false;
}

void ModeModelMeshDeform::on_mouse_move() {
	if (!brushing)
		return;
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->get_float("diameter") / 2;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius * 0.7f) {
		distance = 0;
		apply();
	}
}

void ModeModelMeshDeform::apply() {
	Action *a = get_action();
	data->execute(a);
}






