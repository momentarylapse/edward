/*
 * ModeModelSkeletonAttachVertices.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelSkeletonAttachVertices.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../MultiView/ColorScheme.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../Mesh/Selection/MeshSelectionModePolygon.h"

struct WBrushConfig {
	float exponent;
	string name;

	string get_icon() const {
		Image im;
		int n = 48;
		im.create(n, n, White);
		for (int i=0; i<n; i++)
			for (int j=0; j<n; j++) {
				vector vv = vector((float)i / (float)n - 0.5f, (float)j / (float)n - 0.5f, 0) * 2;
				float dd = vv * vv;
				float a = exp(-pow(dd, exponent)*2);
				im.set_pixel(i, j, a * Black + (1-a) * White);

			}
		return hui::SetImage(&im, "image:paint-brush-" + name);
	}
};

const int NUM_BRUSHES = 6;
const WBrushConfig BRUSH_PARAM[NUM_BRUSHES] = {
		{400.0f, "extra hard"},
		{4.0f, "hard"},
		{2.0f, "medium"},
		{1.0f, "medium soft"},
		{0.5f, "soft"},
		{0.25f, "extra soft"}
};



class WeightBrushPanel : public hui::Panel {
public:
	WeightBrushPanel(ModeModelSkeletonAttachVertices *_mode) {
		mode = _mode;

		base_diameter = mode->multi_view->cam.radius * 0.1f;

		from_resource("model-texture-paint-brush-dialog");

		event("diameter-slider", [=]{ on_diameter_slider(); });
		event("opacity-slider", [=]{ on_opacity_slider(); });
		event("alpha-slider", [=]{ on_alpha_slider(); });

		for (int i=0; i<NUM_BRUSHES; i++)
			add_string("brush-type", BRUSH_PARAM[i].get_icon() + "\\" + BRUSH_PARAM[i].name);
		set_float("diameter-slider", 0.5f);
		set_float("opacity-slider", 1.0f);
		set_float("alpha-slider", 1.0f);

		set_string("diameter", f2s(base_diameter, 2));
		set_float("opacity", 1.0f);
		set_float("alpha", 1.0f);
		set_int("brush-type", 2);
		set_color("color", Red);
		check("scale-by-pressure", true);
		check("opacity-by-pressure", true);
	}

	void on_diameter_slider() {
		float x = get_float("");
		set_string("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
	}

	void on_opacity_slider() {
		float x = get_float("");
		set_float("opacity", x);
	}

	void on_alpha_slider() {
		float x = get_float("");
		set_float("alpha", x);
	}

	ModeModelSkeletonAttachVertices *mode;
	float base_diameter;
};



ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(ModeBase* _parent, int _bone_index) :
	ModeCreation<DataModel>("ModelSkeletonAttachVertices", _parent)
{
	message = _("Select vertices, [Ctrl + Return] = done");
	bone_index = _bone_index;
	brushing = false;
	distance = 1;
}

void ModeModelSkeletonAttachVertices::on_start() {

	dialog = new WeightBrushPanel(this);
	ed->set_side_panel(dialog);

	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	mode_model->allow_selection_modes(false);

	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);

	data->subscribe(this, [=]{ on_data_change(); });

	on_data_change();
}

void ModeModelSkeletonAttachVertices::on_end() {
	data->unsubscribe(this);
	multi_view->unsubscribe(this);

	mode_model->allow_selection_modes(false);

	//parent->on_update(data, "");
}

void ModeModelSkeletonAttachVertices::on_data_change() {
	mode_model_mesh->selection_mode->update_multi_view();
}

void ModeModelSkeletonAttachVertices::on_command(const string &id) {
}


void _draw_weights(MultiView::Window *win, ModelMesh *m, int bone_index) {
	color bg = win->get_background_color();
	auto *multi_view = win->multi_view;

	nix::set_offset(-2);
	set_line_width(scheme.LINE_WIDTH_MEDIUM);
	Array<vector> line_pos;
	Array<color> line_color;

	vector dir, up, right;
	win->get_camera_frame(dir, up, right);
	for (auto &v: m->vertex) {
		float w = 0;
		if (v.bone_index.i == bone_index)
			w = max(w, v.bone_weight.x);
		if (v.bone_index.j == bone_index)
			w = max(w, v.bone_weight.y);
		if (v.bone_index.k == bone_index)
			w = max(w, v.bone_weight.z);
		if (v.bone_index.l == bone_index)
			w = max(w, v.bone_weight.w);
		if (w == 0)
			continue;

		color cc = scheme.SELECTION;
		line_color.add(cc);
		line_color.add(cc);
		line_pos.add(v.pos);
		line_pos.add(v.pos - right * w * win->cam->radius * 0.02f);
	}
	draw_lines_colored(line_pos, line_color, false);
	nix::set_offset(0);
}

void ModeModelSkeletonAttachVertices::on_draw_win(MultiView::Window *win) {
	mode_model_mesh->on_draw_win(win);

	_draw_weights(win, data->mesh, bone_index);

	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	vector n = data->mesh->polygon[multi_view->hover.index].temp_normal;

	set_color(scheme.CREATION_LINE);
	set_line_width(scheme.LINE_WIDTH_MEDIUM);
	float radius = dialog->get_float("diameter") / 2;
	draw_circle(pos, n, radius);
}

float ModeModelSkeletonAttachVertices::radius() {
	float radius = dialog->get_float("diameter") / 2;
	if (dialog->is_checked("scale-by-pressure"))
		radius *= hui::GetEvent()->pressure;
	return radius;
}

void ModeModelSkeletonAttachVertices::on_mouse_move() {
	if (!brushing)
		return;
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius() * 0.7f) {
		distance = 0;
		apply();
	}
}

void ModeModelSkeletonAttachVertices::on_left_button_down() {
	if (multi_view->hover.index < 0)
		return;
	data->begin_action_group("brush");
	vector pos = multi_view->hover.point;
	distance = 0;
	last_pos = pos;
	brushing = true;

	apply();
}

void ModeModelSkeletonAttachVertices::on_left_button_up() {
	if (brushing)
		data->end_action_group();
	brushing = false;
}

void ModeModelSkeletonAttachVertices::on_set_multi_view() {
	mode_model_mesh->on_set_multi_view();
}

int ivec4_find(const ivec4 &v, int x) {
	if (v.i == x)
		return 0;
	if (v.j == x)
		return 1;
	if (v.k == x)
		return 2;
	if (v.l == x)
		return 3;
	return -1;
}

int vec4_min(const vec4 &v) {
	int n = 0;
	float m = v.x;
	if (v.y < m) {
		n = 1;
		m = v.y;
	}
	if (v.z < m) {
		n = 2;
		m = v.z;
	}
	if (v.w < m) {
		n = 3;
		m = v.w;
	}
	return n;
}

void vec4_set(vec4 &v, int i, float x) {
	auto vv = &v.x;
	vv[i] = x;
}

float vec4_get(vec4 &v, int i) {
	auto vv = &v.x;
	return vv[i];
}

void ivec4_set(ivec4 &v, int i, int x) {
	auto vv = &v.i;
	vv[i] = x;
}

float vec4_sum(const vec4 &v) {
	return v.x + v.y + v.z + v.w;
}

static void set_weight(ivec4 &bones, vec4 &weights, int bone, float weight) {
	int prev = ivec4_find(bones, bone);
	if (prev >= 0) {
		// sum(other) != 1-weight
		weights *= (1-weight) / max(vec4_sum(weights) - vec4_get(weights, prev), 0.001f);
		vec4_set(weights, prev, weight);
	} else {
		prev = vec4_min(weights);
		weights *= (1-weight) / max(vec4_sum(weights) - vec4_get(weights, prev), 0.001f);
		vec4_set(weights, prev, weight);
		ivec4_set(bones, prev, bone);
	}

	weights /= vec4_sum(weights);
}

void ModeModelSkeletonAttachVertices::apply() {

	vector pos = multi_view->hover.point;
	vector n = data->mesh->polygon[multi_view->hover.index].temp_normal;
	int type = dialog->get_int("brush-type");
	float weight0 = dialog->get_float("alpha");
	float opacity = dialog->get_float("opacity");
	if (dialog->is_checked("opacity-by-pressure"))
		weight0 *= hui::GetEvent()->pressure;
	float R = radius();
	auto bp = BRUSH_PARAM[type];

	Array<int> index;
	Array<ivec4> bone;
	Array<vec4> bone_weight;
	foreachi(auto &v, data->edit_mesh->vertex, i) {
		float r = (v.pos - pos).length();
		if (r > R)
			continue;
		auto bb = v.bone_index;
		auto ww = v.bone_weight;
		set_weight(bb, ww, bone_index, weight0 * exp(-pow(r/R, bp.exponent)*2));
		index.add(i);
		bone.add(bb);
		bone_weight.add(ww);
	}
	if (index.num > 0)
		data->boneAttachVertices(index, bone, bone_weight);
}
