/*
 * ModeModelMeshPaint.cpp
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#include "ModeModelMeshPaint.h"
#include "ModeModelMesh.h"
#include "../ModeModel.h"
#include "../../../multiview/MultiView.h"
#include "../../../multiview/DrawingHelper.h"
#include "../../../multiview/ColorScheme.h"
#include "../../../stuff/BrushPanel.h"
#include "../../../Edward.h"
#include "../../../lib/nix/nix.h"
#include "../../../lib/math/complex.h"
#include "../../../action/model/mesh/brush/ActionModelBrushExtrude.h"
#include "../../../action/model/mesh/brush/ActionModelBrushSmooth.h"
#include "../../../action/model/mesh/brush/ActionModelBrushComplexify.h"

ModeModelMeshPaint *mode_model_mesh_paint = NULL;




// map from 3d-world-space into UV-space for a polygon
// D: Tp -> Tuv
vec3 map_uv(ModelMesh *m, int poly, const vec3 &pos, mat4 &D) {
	auto &p = m->polygon[poly];

	for (int k=0; k<p.side.num-2; k++) {
		int ta = p.side[k].triangulation[0];
		int tb = p.side[k].triangulation[1];
		int tc = p.side[k].triangulation[2];
		vec3 a = m->vertex[p.side[ta].vertex].pos;
		vec3 b = m->vertex[p.side[tb].vertex].pos;
		vec3 c = m->vertex[p.side[tc].vertex].pos;
		vec3 sa = p.side[ta].skin_vertex[0];
		vec3 sb = p.side[tb].skin_vertex[0];
		vec3 sc = p.side[tc].skin_vertex[0];
		auto fg = bary_centric(pos, a, b, c);
		if (fg.x >= 0 and fg.y >= 0 and fg.x + fg.y <= 1) {
			D = mat4(sb-sa, sc-sa, v_0) * mat4(b-a, c-a, vec3::cross(b-a, c-a)).inverse();
			//D = matrix::ID;
			return sa + fg.x * (sb - sa) + fg.y * (sc - sa);
		}
	}
	return v_0;
}

mat4 inv_2d(const mat4 &DD) {
	float det = DD._00 * DD._11 - DD._01 * DD._10;
	mat4 i;
	i._00 = DD._11;
	i._01 = -DD._10;
	i._10 = -DD._01;
	i._11 = DD._00;
	return i * (1 / det);
}

float quad_2d(const mat4 &DD, const vec3 &v) {
	return (v.x*v.x*DD._00 + v.y*v.y*DD._11 + 2*v.x*v.y*DD._01);
}

complex eigen_value_2d(const mat4 &m) {
	float d = sqrt( (m._00 + m._11)*(m._00 + m._11) - 4*(m._00*m._11 - m._01*m._01));
	return complex(m._00+m._11 + d, m._00+m._11 - d) / 2;
}


class ActionModelBrushTexturePaint: public Action {
public:
	ActionModelBrushTexturePaint(const vec3 &_n, int _poly, const BrushState &_brush) {
		n = _n;
		poly = _poly;
		brush = _brush;
	}
	string name(){	return "ModelBrushTexturePaint";	}

	void *execute(Data *d) {
		DataModel *m = dynamic_cast<DataModel*>(d);

		auto *tl = m->material[m->mesh->polygon[poly].material]->texture_levels[0];

		vec3 e1 = n.ortho();
		vec3 e2 = n ^ e1;

		mat4 D;
		vec3 v = map_uv(m->mesh, poly, brush.m0, D);

		auto DD = D * D.transpose();
		auto iDD = inv_2d(DD);

		float threshold = 0.001f;

		// maximal pixel area from ellipsis
		auto ev = eigen_value_2d(DD);
		float rmax = sqrt(max(ev.x, ev.y)) * brush.R * 1.2f;
		if (brush.exponent < 1.2f)
			rmax *= 2;
		if (brush.exponent < 0.8f)
			rmax *= 5;

		// pixel
		int i0 = clamp(int(v.x * tl->image->width), 0, tl->image->width);
		int j0 = clamp(int(v.y * tl->image->height), 0, tl->image->height);
		int di = rmax * tl->image->width;
		int dj = rmax * tl->image->height;


		float rr = brush.R * brush.R;

		// draw a fuzzy ellipsis UV-space
		for (int ii=i0-di; ii<i0+di; ii++)
			for (int jj=j0-dj; jj<j0+dj; jj++) {
				int i = loop(ii, 0, tl->image->width);
				int j = loop(jj, 0, tl->image->height);
				vec3 vv = vec3((float)ii / (float)tl->image->width, (float)jj / (float)tl->image->height, 0);
				float dd = quad_2d(iDD, vv-v);
				float a = exp(-pow(dd / rr, brush.exponent)*2) * brush.opacity;
				if (a > threshold) {
					color c = tl->image->get_pixel(i, j);
					tl->image->set_pixel(i, j, (1-a) * c + a * brush.col);
				}
			}

		tl->edited = true;
		tl->update_texture();
		m->out_texture_changed();

		return NULL;
	}
	void undo(Data *d) {
	}

private:
	vec3 n;
	int poly;
	BrushState brush;
};





ModeModelMeshPaint::ModeModelMeshPaint(ModeBase *_parent, MultiView::MultiView *mv) :
		Mode<DataModel>("ModelMeshPaint", _parent, mv, "menu_model") {
	dialog = nullptr;
	brushing = false;
	distance = 1;
}

void ModeModelMeshPaint::on_start() {

	dialog = new BrushPanel(multi_view, "model-texture-paint-brush-dialog");
	ed->set_side_panel(dialog);

	auto *t = ed->get_toolbar(hui::TOOLBAR_LEFT);
	t->reset();
	t->enable(false);
	multi_view->set_allow_action(false);

	// enter
	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	ed->mode_model->allow_selection_modes(false);
	mode_model_mesh->set_allow_draw_hover(false);



	multi_view->set_allow_select(false);
}

void ModeModelMeshPaint::on_end() {
	if (brushing)
		data->end_action_group();

	ed->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->set_allow_action(true);
	ed->mode_model->allow_selection_modes(true);
	mode_model_mesh->set_allow_draw_hover(true);
	ed->set_side_panel(nullptr);
}

void ModeModelMeshPaint::on_set_multi_view() {
	parent->on_set_multi_view();
}

void ModeModelMeshPaint::on_draw() {
	parent->on_draw();
}

void ModeModelMeshPaint::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (multi_view->hover.index < 0)
		return;
	vec3 pos = multi_view->hover.point;
	vec3 n = data->mesh->polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->radius0();

	set_color(scheme.CREATION_LINE);
	set_line_width(scheme.LINE_WIDTH_MEDIUM);
	draw_circle(pos, n, radius);
}

void ModeModelMeshPaint::on_selection_change() {
	parent->on_selection_change();
}

float ModeModelMeshPaint::radius() {
	float radius = dialog->get_float("diameter") / 2;
	if (dialog->is_checked("scale-by-pressure"))
		radius *= hui::get_event()->pressure;
	return radius;
}

Action *ModeModelMeshPaint::get_action() {
	vec3 pos = multi_view->hover.point;
	vec3 n = data->mesh->polygon[multi_view->hover.index].temp_normal;

	auto brush = dialog->prepare(pos);

	return new ActionModelBrushTexturePaint(n, multi_view->hover.index, brush);
}

void ModeModelMeshPaint::on_left_button_down() {
	if (multi_view->hover.index < 0)
		return;
	data->begin_action_group("brush");
	vec3 pos = multi_view->hover.point;
	distance = 0;
	last_pos = pos;
	brushing = true;

	apply();
}

void ModeModelMeshPaint::on_left_button_up() {
	if (brushing)
		data->end_action_group();
	brushing = false;
}

void ModeModelMeshPaint::on_mouse_move() {
	if (!brushing)
		return;
	if (multi_view->hover.index < 0)
		return;
	vec3 pos = multi_view->hover.point;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius() * 0.7f) {
		distance = 0;
		apply();
	}
}

void ModeModelMeshPaint::apply() {
	Action *a = get_action();
	data->execute(a);
}






