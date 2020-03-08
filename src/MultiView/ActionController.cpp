/*
 * ActionController.cpp
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#include "ActionController.h"
#include "MultiView.h"
#include "DrawingHelper.h"
#include "Window.h"
#include "../Action/ActionMultiView.h"
#include "../lib/nix/nix.h"
#include "../Data/Model/Geometry/Geometry.h"
#include "../Data/Model/Geometry/GeometryBall.h"
#include "../Data/Model/Geometry/GeometryCylinder.h"
#include "../Data/Model/Geometry/GeometryTorus.h"
#include "../Data/Model/Geometry/GeometryCube.h"

#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))


namespace MultiView {


ActionController::ActionController(MultiView *view) {
	multi_view = view;
	geo_mat = matrix::ID;
	mat = matrix::ID;


	float r0 = 1.333f;
	float r1 = 0.666f;
	float r = 0.1f;
	geo.add(new GeometryTorus(v_0, vector::EZ, 1.0f, r, 32, 8));
	geo.add(new GeometryTorus(v_0, vector::EY, 1.0f, r, 32, 8));
	geo.add(new GeometryTorus(v_0, vector::EX, 1.0f, r, 32, 8));
	geo.add(new GeometryCylinder(-vector::EX*r0, -vector::EX*r1, r, 1, 8));
	geo.add(new GeometryCylinder( vector::EX*r0,  vector::EX*r1, r, 1, 8));
	geo.add(new GeometryCylinder(-vector::EY*r0, -vector::EY*r1, r, 1, 8));
	geo.add(new GeometryCylinder( vector::EY*r0,  vector::EY*r1, r, 1, 8));
	geo.add(new GeometryCylinder(-vector::EZ*r0, -vector::EZ*r1, r, 1, 8));
	geo.add(new GeometryCylinder( vector::EZ*r0,  vector::EZ*r1, r, 1, 8));
	r = 0.03f;
	geo_show.add(new GeometryTorus(v_0, vector::EZ, 1.0f, r, 32, 8));
	geo_show.add(new GeometryTorus(v_0, vector::EY, 1.0f, r, 32, 8));
	geo_show.add(new GeometryTorus(v_0, vector::EX, 1.0f, r, 32, 8));
	geo_show.add(new GeometryCylinder(-vector::EX*r0, -vector::EX*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder( vector::EX*r0,  vector::EX*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder(-vector::EY*r0, -vector::EY*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder( vector::EY*r0,  vector::EY*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder(-vector::EZ*r0, -vector::EZ*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder( vector::EZ*r0,  vector::EZ*r1, r, 1, 8));

	for (auto g: geo_show){
		auto *vb = new nix::VertexBuffer("3f,3fn,2f");
		g->build(vb);
		buf.add(vb);
	}

	reset();
}

ActionController::~ActionController() {
	delete_geo();
}

void ActionController::start_action(Window *_win, const vector &_m, int _constraints) {
	if (cur_action)
		end_action(false);
	if (!multi_view->allow_mouse_actions)
		return;
	if (action.name == "")
		return;

	mat = matrix::ID;
	active_win = _win;
	m0 = _m;
	pos0 = pos;
	constraints = _constraints;
	if (constraints == ACTION_CONSTRAINTS_NONE)
		pos0 = m0;
	cur_action = ActionMultiViewFactory(action.name, data);
	cur_action->execute_logged(data);
	multi_view->notify(multi_view->MESSAGE_ACTION_START);
}


vector transform_ang(Window *w, const vector &ang) {
	auto qmv =  w->local_ang;
	auto qang = quaternion::rotation_v( ang);
	auto q = qmv * qang * qmv.bar();
	return q.get_angles();
}

vector mvac_project_trans(int mode, const vector &v) {
	vector r = v;
	if (mode == ACTION_CONSTRAINTS_X)
		r.y = r.z = 0;
	else if (mode == ACTION_CONSTRAINTS_Y)
		r.x = r.z = 0;
	else if (mode == ACTION_CONSTRAINTS_Z)
		r.x = r.y = 0;
	else if (mode == ACTION_CONSTRAINTS_XY)
		r.z = 0;
	else if (mode == ACTION_CONSTRAINTS_XZ)
		r.y = 0;
	else if (mode == ACTION_CONSTRAINTS_YZ)
		r.x = 0;
	return r;
}

vector mvac_mirror(int mode) {
	if (mode == ACTION_CONSTRAINTS_X)
		return vector::EX;
	else if (mode == ACTION_CONSTRAINTS_Y)
		return vector::EY;
	else if (mode == ACTION_CONSTRAINTS_Z)
		return vector::EZ;
	else if (mode == ACTION_CONSTRAINTS_XY)
		return vector::EZ;
	else if (mode == ACTION_CONSTRAINTS_XZ)
		return vector::EY;
	return vector::EX;
}

void ActionController::update_action() {
	if (!cur_action)
		return;

	vector v2p = multi_view->m;
	vector v2  = active_win->unproject(v2p, m0);
	vector v1  = m0;
	vector v1p = active_win->project(v1);
	vector dir = active_win->get_direction();
	vector _param = v_0;
	if (action.mode == ACTION_MOVE) {
		_param = mvac_project_trans(constraints, v2 - v1);
		_param = multi_view->maybe_snap_v(_param);
	} else if (action.mode == ACTION_ROTATE) {
		//_param = mvac_project_trans(constraints, v2 - v1) * 0.003f * multi_view->active_win->zoom();
		_param = mvac_project_trans(constraints, (v2 - v1) ^ dir) * 0.003f * multi_view->active_win->zoom();
		if (constraints == ACTION_CONSTRAINTS_NONE)
			_param = transform_ang(active_win, vector(v1p.y - v2p.y, v1p.x - v2p.x, 0) * 0.003f);
		_param = multi_view->maybe_snap_v2(_param, pi / 180.0);
	} else if (action.mode == ACTION_SCALE) {
		_param = vector(1, 1, 1) + mvac_project_trans(constraints, v2 - v1) * 0.01f * multi_view->active_win->zoom();
		if (constraints == ACTION_CONSTRAINTS_NONE)
			_param = vector(1, 1, 1) * (1 + (v2p - v1p).x * 0.01f);
		_param = multi_view->maybe_snap_v2(_param, 0.01f);
	} else if (action.mode == ACTION_MIRROR) {
		_param = mvac_mirror(constraints);
		if (constraints == ACTION_CONSTRAINTS_NONE)
			_param = active_win->cam->ang * vector::EX;
	} else {
		param = v_0;
	}
	update_param(_param);
}

void ActionController::update_param(const vector &_param) {
	if (!cur_action)
		return;

	auto m_dt = matrix::translation(pos0);
	auto m_dti = matrix::translation(-pos0);

	param = _param;
	if (action.mode == ACTION_MOVE) {
		mat = matrix::translation(param);
	} else if (action.mode == ACTION_ROTATE) {
		mat = matrix::rotation(param);
		mat = m_dt * mat * m_dti;
	} else if (action.mode == ACTION_SCALE) {
		mat = matrix::scale(param.x, param.y, param.z);
		mat = m_dt * mat * m_dti;
	} else if (action.mode == ACTION_MIRROR) {
		plane pl = plane::from_point_normal(v_0, param);
		mat = matrix::reflection(pl);
		mat = m_dt * mat * m_dti;
	} else {
		mat = matrix::ID;
	}
	cur_action->update_and_notify(data, mat);

	update();

	multi_view->notify(multi_view->MESSAGE_ACTION_UPDATE);
}



void ActionController::end_action(bool set) {
	if (!cur_action)
		return;
	if (set) {
		cur_action->undo(data);
		data->execute(cur_action);
		multi_view->notify(multi_view->MESSAGE_ACTION_EXECUTE);
	} else {
		cur_action->abort_and_notify(data);
		delete(cur_action);
		multi_view->notify(multi_view->MESSAGE_ACTION_ABORT);
	}
	cur_action = NULL;
	mat = matrix::ID;
}

bool ActionController::is_selecting() {
	if (action.mode == ACTION_SELECT)
		return true;
	if (!action.locked)
		return !multi_view->hover_selected();
	return false;
}

void ActionController::reset() {
	visible = false;
	constraints = ACTION_CONSTRAINTS_NONE;
	mouse_over_constraint = -1;
}

void ActionController::delete_geo() {
	for (Geometry *g: geo)
		delete(g);
	geo.clear();
	for (Geometry *g: geo_show)
		delete(g);
	geo_show.clear();
}

void ActionController::update() {
	if (cur_action) {
		pos = pos0;
	} else {
		pos = multi_view->get_selection_center();
	}
	float f = multi_view->cam.radius * 0.15f;
	if (multi_view->whole_window)
		f /= 2;
	auto s = matrix::scale(f, f, f);
	auto t = matrix::translation(pos);
	geo_mat = t * s;

	multi_view->force_redraw();
}

void ActionController::show(bool show) {
	visible = show;
	update();
}

struct ACGeoConfig {
	color col;
	int constraint;
	int priority;
};

string constraint_name(int c) {
	if (c == ACTION_CONSTRAINTS_X)
		return _("x-axis");
	if (c == ACTION_CONSTRAINTS_Y)
		return _("y-axis");
	if (c == ACTION_CONSTRAINTS_Z)
		return _("z-axis");
	if (c == ACTION_CONSTRAINTS_XY)
		return _("x-y-plane");
	if (c == ACTION_CONSTRAINTS_XZ)
		return _("x-z-plane");
	if (c == ACTION_CONSTRAINTS_YZ)
		return _("y-z-plane");
	return "free";
}

string action_name(int a) {
	if (a == ACTION_MOVE)
		return _("move");
	if ((a == ACTION_ROTATE) or (a == ACTION_ROTATE_2D))
		return _("rotate");
	if ((a == ACTION_SCALE) or (a == ACTION_SCALE_2D))
		return _("scale");
	if (a == ACTION_MIRROR)
		return _("reflect");
	return "???";
}

const ACGeoConfig ac_geo_config[] = {
	{color(1, 0.4f, 0.4f, 0.8f),ACTION_CONSTRAINTS_XY,0},
	{color(1, 0.4f, 0.4f, 0.8f),ACTION_CONSTRAINTS_XZ,0},
	{color(1, 0.4f, 0.4f, 0.8f),ACTION_CONSTRAINTS_YZ,0},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_X,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_X,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Y,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Y,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Z,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Z,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_NONE,2}
};

bool geo_allow(int i, Window *win, const matrix &geo_mat) {
	int c = ac_geo_config[i].constraint;
	vector pp = win->project(geo_mat * v_0);
	vector ppx = win->project(geo_mat * vector::EX);
	ppx.z = pp.z;
	vector ppy = win->project(geo_mat * vector::EY);
	ppy.z = pp.z;
	vector ppz = win->project(geo_mat * vector::EZ);
	ppz.z = pp.z;

	if (c == ACTION_CONSTRAINTS_X)
		return (ppx - pp).length() > 8;
	if (c == ACTION_CONSTRAINTS_Y)
		return (ppy - pp).length() > 8;
	if (c == ACTION_CONSTRAINTS_Z)
		return (ppz - pp).length() > 8;
	if (c == ACTION_CONSTRAINTS_YZ)
		return ((ppy - pp) ^ (ppz - pp)).length() > 300;
	if (c == ACTION_CONSTRAINTS_XZ)
		return ((ppx - pp) ^ (ppz - pp)).length() > 300;
	if (c == ACTION_CONSTRAINTS_XY)
		return ((ppx - pp) ^ (ppy - pp)).length() > 300;
	return true;
}

// in 2d mode!
void ActionController::draw(Window *win) {
	if (!multi_view->allow_mouse_actions)
		return;
	if (!visible)
		return;
	nix::SetZ(false, false);
	matrix m = mat * geo_mat;
	nix::SetWorldMatrix(m);
	nix::SetTexture(NULL);
	nix::SetShader(nix::default_shader_3d);
	win->set_projection_matrix();
	foreachi(Geometry *g, geo_show, i) {
		if (!geo_allow(i, win, m))
			continue;
		if (ac_geo_config[i].constraint == mouse_over_constraint)
			nix::SetMaterial(Black, White, Black, 0, White);
		else
			nix::SetMaterial(Black, Black, Black, 0, ac_geo_config[i].col);
		nix::DrawTriangles(buf[i]);
	}
	nix::SetZ(false, false);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetWorldMatrix(matrix::ID);
	win->set_projection_matrix_pixel();

	if (in_use()) {
		set_color(color(1, 0.2f, 0.7f, 0.2f));
		set_line_width(2.0f);
		float r = multi_view->cam.radius * 10;
		if (constraints == ACTION_CONSTRAINTS_X)
			draw_line(pos - vector::EX * r, pos + vector::EX * r);
		if (constraints == ACTION_CONSTRAINTS_Y)
			draw_line(pos - vector::EY * r, pos + vector::EY * r);
		if (constraints == ACTION_CONSTRAINTS_Z)
			draw_line(pos - vector::EZ * r, pos + vector::EZ * r);
	}

	nix::SetShader(nix::default_shader_2d);

	if (win == multi_view->mouse_win) {
		vector pp = win->project(pos);

		if ((mouse_over_constraint >= 0) and !in_use()) {
			draw_str(pp.x + 80, pp.y + 40, action_name(action.mode) + ": " + constraint_name(mouse_over_constraint));
		}
	}

	if (in_use() and (win == multi_view->active_win)) {
		vector pp = win->project(pos);

		float x0 = pp.x + 120;//multi_view->m.x + 100;//win->dest.x1 + 120;
		float y0 = pp.y + 40;//multi_view->m.y + 50;//win->dest.y1 + 100;

		string s;
		if (action.mode == ACTION_MOVE) {
			vector t = param;
			string unit = multi_view->get_unit_by_zoom(t);
			s = f2s(t.x, 2) + " " + unit + "\n" + f2s(t.y, 2) + " " + unit;
			if (multi_view->mode3d)
				s += "\n" + f2s(t.z, 2) + " " + unit;
		} else if ((action.mode == ACTION_ROTATE) or (action.mode == ACTION_ROTATE_2D)) {
			vector r = param * 180.0f / pi;
			s = format("%.1f°\n%.1f°\n%.1f°", r.x, r.y, r.z);
		} else if ((action.mode == ACTION_SCALE) or (action.mode == ACTION_SCALE_2D)) {
			if (multi_view->mode3d)
				s = format("%.1f%%\n%.1f%%\n%.1f%%", param.x*100, param.y*100, param.z*100);
			else
				s = format("%.1f%%\n%.1f%%", param.x*100, param.y*100);
		}
		draw_str(x0, y0, s, TextAlign::RIGHT);
	}
}

void ActionController::draw_post() {
}

bool ActionController::is_mouse_over(vector &tp) {
	mouse_over_constraint = -1;
	if (!visible)
		return false;
	float z_min = 1;
	int priority = -1;
	foreachi(Geometry *g, geo, i) {
		vector t;
		if (!geo_allow(i, multi_view->mouse_win, geo_mat))
			continue;
		if (g->is_mouse_over(multi_view->mouse_win, geo_mat, t)) {
			float z = multi_view->mouse_win->project(t).z;
			if ((z < z_min) or (ac_geo_config[i].priority >= priority)) {
				mouse_over_constraint = ac_geo_config[i].constraint;
				priority = ac_geo_config[i].priority;
				z_min = z;
				tp = t;
			}
		}
	}
	return (mouse_over_constraint >= 0);
}

bool ActionController::on_left_button_down() {
	if (!visible and action.locked)
		return false;
	vector hp = multi_view->hover.point;
	if (is_mouse_over(hp)) {
		start_action(multi_view->active_win, hp, mouse_over_constraint);
		return true;
	}
	if (multi_view->hover.index >= 0){
		start_action(multi_view->active_win, hp, ACTION_CONSTRAINTS_NONE);
		return true;
	}
	return false;
}

void ActionController::on_left_button_up() {
	end_action(true);

	update();
}

bool ActionController::in_use() {
	return cur_action;
}

void ActionController::on_mouse_move() {
	update_action();
}

};
