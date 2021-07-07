/*
 * ModeModelMeshPaint.cpp
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#include "ModeModelMeshPaint.h"
#include "ModeModelMesh.h"
#include "../ModeModel.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/DrawingHelper.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../Stuff/BrushPanel.h"
#include "../../../Edward.h"
#include "../../../lib/nix/nix.h"
#include "../../../lib/math/complex.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushExtrude.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushSmooth.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushComplexify.h"

ModeModelMeshPaint *mode_model_mesh_paint = NULL;




// map from 3d-world-space into UV-space for a polygon
// D: Tp -> Tuv
vector map_uv(ModelMesh *m, int poly, const vector &pos, matrix &D) {
	auto &p = m->polygon[poly];

	for (int k=0; k<p.side.num-2; k++) {
		int ta = p.side[k].triangulation[0];
		int tb = p.side[k].triangulation[1];
		int tc = p.side[k].triangulation[2];
		vector a = m->vertex[p.side[ta].vertex].pos;
		vector b = m->vertex[p.side[tb].vertex].pos;
		vector c = m->vertex[p.side[tc].vertex].pos;
		vector sa = p.side[ta].skin_vertex[0];
		vector sb = p.side[tb].skin_vertex[0];
		vector sc = p.side[tc].skin_vertex[0];
		float f, g;
		GetBaryCentric(pos, a, b, c, f, g);
		if (f >= 0 and g >= 0 and f+g <= 1) {
			D = matrix(sb-sa, sc-sa, v_0) * matrix(b-a, c-a, vector::cross(b-a, c-a)).inverse();
			//D = matrix::ID;
			return sa + f * (sb - sa) + g * (sc - sa);
		}
	}
	return v_0;
}

matrix inv_2d(const matrix &DD) {
	float det = DD._00 * DD._11 - DD._01 * DD._10;
	matrix i;
	i._00 = DD._11;
	i._01 = -DD._10;
	i._10 = -DD._01;
	i._11 = DD._00;
	return i * (1 / det);
}

float quad_2d(const matrix &DD, const vector &v) {
	return (v.x*v.x*DD._00 + v.y*v.y*DD._11 + 2*v.x*v.y*DD._01);
}

complex eigen_value_2d(const matrix &m) {
	float d = sqrt( (m._00 + m._11)*(m._00 + m._11) - 4*(m._00*m._11 - m._01*m._01));
	return complex(m._00+m._11 + d, m._00+m._11 - d) / 2;
}


class ActionModelBrushTexturePaint: public Action {
public:
	ActionModelBrushTexturePaint(const vector &_n, int _poly, const BrushState &_brush) {
		n = _n;
		poly = _poly;
		brush = _brush;
	}
	string name(){	return "ModelBrushTexturePaint";	}

	void *execute(Data *d) {
		DataModel *m = dynamic_cast<DataModel*>(d);

		auto *tl = m->material[m->mesh->polygon[poly].material]->texture_levels[0];

		vector e1 = n.ortho();
		vector e2 = n ^ e1;

		matrix D;
		vector v = map_uv(m->mesh, poly, brush.m0, D);

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
				vector vv = vector((float)ii / (float)tl->image->width, (float)jj / (float)tl->image->height, 0);
				float dd = quad_2d(iDD, vv-v);
				float a = exp(-pow(dd / rr, brush.exponent)*2) * brush.opacity;
				if (a > threshold) {
					color c = tl->image->get_pixel(i, j);
					tl->image->set_pixel(i, j, (1-a) * c + a * brush.col);
				}
			}

		tl->edited = true;
		tl->update_texture();
		m->notify(m->MESSAGE_TEXTURE_CHANGE);

		return NULL;
	}
	void undo(Data *d) {
	}

private:
	vector n;
	int poly;
	BrushState brush;
};





ModeModelMeshPaint::ModeModelMeshPaint(ModeBase *_parent) :
		Mode<DataModel>("ModelMeshPaint", _parent, ed->multi_view_3d, "menu_model") {
	dialog = nullptr;
	brushing = false;
	distance = 1;
}

void ModeModelMeshPaint::on_start() {

	dialog = new BrushPanel(multi_view, "model-texture-paint-brush-dialog");
	ed->set_side_panel(dialog);

	auto *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);
	multi_view->set_allow_action(false);

	// enter
	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	mode_model->allow_selection_modes(false);
	mode_model_mesh->set_allow_draw_hover(false);



	multi_view->set_allow_select(false);
}

void ModeModelMeshPaint::on_end() {
	if (brushing)
		data->end_action_group();

	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->set_allow_action(true);
	mode_model->allow_selection_modes(true);
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
	vector pos = multi_view->hover.point;
	vector n = data->mesh->polygon[multi_view->hover.index].temp_normal;
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
		radius *= hui::GetEvent()->pressure;
	return radius;
}

Action *ModeModelMeshPaint::get_action() {
	vector pos = multi_view->hover.point;
	vector n = data->mesh->polygon[multi_view->hover.index].temp_normal;

	auto brush = dialog->prepare(pos);

	return new ActionModelBrushTexturePaint(n, multi_view->hover.index, brush);
}

void ModeModelMeshPaint::on_left_button_down() {
	if (multi_view->hover.index < 0)
		return;
	data->begin_action_group("brush");
	vector pos = multi_view->hover.point;
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
	vector pos = multi_view->hover.point;
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






