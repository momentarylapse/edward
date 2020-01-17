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
#include "../../../Edward.h"
#include "../../../lib/nix/nix.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushExtrude.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushSmooth.h"
#include "../../../Action/Model/Mesh/Brush/ActionModelBrushComplexify.h"

ModeModelMeshPaint *mode_model_mesh_paint = NULL;




// map from 3d-world-space into UV-space for a polygon
// D: Tp -> Tuv
vector map_uv(DataModel *m, int surf, int poly, const vector &pos, matrix &D) {
	auto &p = m->surface[surf].polygon[poly];

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


class ActionModelBrushTexturePaint: public Action {
public:
	ActionModelBrushTexturePaint(const vector &_pos, const vector &_n, float _radius, int _surf, int _poly, const color &_col) {
		pos = _pos;
		n = _n;
		radius = _radius;
		surf = _surf;
		poly = _poly;
		col = _col;
	}
	string name(){	return "ModelBrushTexturePaint";	}

	void *execute(Data *d) {
		DataModel *m = dynamic_cast<DataModel*>(d);

		auto *tl = m->material[0]->texture_levels[0];

		vector e1 = n.ortho();
		vector e2 = n ^ e1;

		matrix D;
		vector v = map_uv(m, surf, poly, pos, D);
		int xx = clampi(v.x * tl->image->width, 0, tl->image->width);
		int yy = clampf(v.y * tl->image->height, 0, tl->image->height);

		vector f1 = D.transform_normal(e1);
		vector f2 = D.transform_normal(e2);
		float rr = (f1.length_sqr() + f2.length_sqr()) * radius * radius;
		int dx = sqrt(rr) * tl->image->width * 1.1f;
		int dy = sqrt(rr) * tl->image->height * 1.1f;
		int x0 = max(xx - dx, 0);
		int y0 = max(yy - dy, 0);
		int x1 = min(xx + dx, tl->image->width);
		int y1 = min(yy + dy, tl->image->height);

		// just draw a fuzzy circle in UV-space...
		for (int i=x0; i<x1; i++)
			for (int j=y0; j<y1; j++) {
				vector vv = vector((float)i / (float)tl->image->width, (float)j / (float)tl->image->height, 0);
				float dd = (vv - v).length_sqr();
				float a = exp(-pow(dd / rr, 4)*2) * col.a;
				if (a > 0.001f) {
					color c = tl->image->get_pixel(i, j);
					tl->image->set_pixel(i, j, (1-a) * c + a * col);
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
	vector pos, n;
	float radius;
	int surf, poly;
	color col;
};


class PaintBrushPanel : public hui::Panel {
public:
	PaintBrushPanel(ModeModelMeshPaint *_mode) {
		mode = _mode;

		base_diameter = mode->multi_view->cam.radius * 0.1f;

		from_resource("model-texture-paint-brush-dialog");

		event("diameter-slider", [=]{ on_diameter_slider(); });
		event("opacity-slider", [=]{ on_opacity_slider(); });

		add_string("brush-type", _("In Textur malen"));
		set_float("diameter-slider", 0.5f);
		set_float("opacity-slider", 1.0f);

		set_string("diameter", f2s(base_diameter, 2));
		set_float("opacity", 1.0f);
		set_int("brush-type", 0);
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

	ModeModelMeshPaint *mode;
	float base_diameter;
};



ModeModelMeshPaint::ModeModelMeshPaint(ModeBase *_parent) :
		Mode<DataModel>("ModelMeshPaint", _parent, ed->multi_view_3d, "menu_model"),
		Observable("ModelMeshPaint") {
	dialog = nullptr;
	brushing = false;
	distance = 1;
}

ModeModelMeshPaint::~ModeModelMeshPaint() {
}

void ModeModelMeshPaint::on_start() {

	dialog = new PaintBrushPanel(this);
	ed->embed(dialog, "root-table", 1, 0);

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

void ModeModelMeshPaint::on_end() {
	if (brushing)
		data->end_action_group();

	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->set_allow_action(true);
	mode_model->allow_selection_modes(true);
	mode_model_mesh->set_allow_draw_hover(true);
	delete dialog;
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
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	int type = dialog->get_int("brush-type");
	auto col = dialog->get_color("color");
	col.a = dialog->get_float("opacity");
	if (dialog->is_checked("opacity-by-pressure"))
		col.a *= hui::GetEvent()->pressure;

	Action *a = NULL;
	if (type == 0)
		a = new ActionModelBrushTexturePaint(pos, n, radius(), multi_view->hover.set, multi_view->hover.index, col);
	return a;
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






