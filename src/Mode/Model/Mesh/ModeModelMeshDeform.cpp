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
#include "../../../Edward.h"
#include "../../../lib/nix/nix.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushExtrude.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushSmooth.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushComplexify.h"

ModeModelMeshDeform *mode_model_mesh_deform = NULL;


class DeformBrushPanel : public hui::Panel {
public:
	DeformBrushPanel(ModeModelMeshDeform *_mode) {
		mode = _mode;

		base_diameter = mode->multi_view->cam.radius * 0.2f;
		base_depth = mode->multi_view->cam.radius * 0.02f;

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

	ModeModelMeshDeform *mode;
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

	dialog = new DeformBrushPanel(this);
	ed->embed(dialog, "root-table", 1, 0);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);
	multi_view->setAllowAction(false);

	// enter
	mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_polygon);
	mode_model->allowSelectionModes(false);



	multi_view->setAllowSelect(false);
}

void ModeModelMeshDeform::on_end() {
	if (brushing)
		data->end_action_group();

	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->setAllowAction(true);
	mode_model->allowSelectionModes(true);
	delete dialog;
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

	nix::SetColor(multi_view->ColorCreationLine);
	MultiView::set_wide_lines(2);
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






