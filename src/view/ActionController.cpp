/*
 * ActionController.cpp
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#include "ActionController.h"
#include "MultiView.h"
//#include "DrawingHelper.h"
//#include "ColorScheme.h"
//#include "Window.h"
//#include "MouseWrapper.h"
#include "EdwardWindow.h"
#include <Session.h>
#include <action/ActionMultiView.h>
#include <data/geometry/Geometry.h>
#include <data/geometry/GeometryBall.h>
#include <data/geometry/GeometryCylinder.h>
#include <data/geometry/GeometryTorus.h>
#include <data/geometry/GeometryCube.h>
#include <y/graphics-impl.h>
#include <y/renderer/world/geometry/RenderViewData.h>
#include <lib/math/plane.h>

#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ActionController::ActionController(MultiView *view) {
	multi_view = view;
	geo_mat = mat4::ID;
	mat = mat4::ID;


	float r0 = 1.333f;
	float r1 = 0.666f;
	float r = 0.1f;
	geo.add(new GeometryTorus(v_0, vec3::EZ, 1.0f, r, 32, 8));
	geo.add(new GeometryTorus(v_0, vec3::EY, 1.0f, r, 32, 8));
	geo.add(new GeometryTorus(v_0, vec3::EX, 1.0f, r, 32, 8));
	geo.add(new GeometryCylinder(-vec3::EX*r0, -vec3::EX*r1, r, 1, 8));
	geo.add(new GeometryCylinder( vec3::EX*r0,  vec3::EX*r1, r, 1, 8));
	geo.add(new GeometryCylinder(-vec3::EY*r0, -vec3::EY*r1, r, 1, 8));
	geo.add(new GeometryCylinder( vec3::EY*r0,  vec3::EY*r1, r, 1, 8));
	geo.add(new GeometryCylinder(-vec3::EZ*r0, -vec3::EZ*r1, r, 1, 8));
	geo.add(new GeometryCylinder( vec3::EZ*r0,  vec3::EZ*r1, r, 1, 8));
	r = 0.015f;
	geo_show.add(new GeometryTorus(v_0, vec3::EZ, 1.0f, r, 32, 8));
	geo_show.add(new GeometryTorus(v_0, vec3::EY, 1.0f, r, 32, 8));
	geo_show.add(new GeometryTorus(v_0, vec3::EX, 1.0f, r, 32, 8));
	r = 0.03f;
	geo_show.add(new GeometryCylinder(-vec3::EX*r0, -vec3::EX*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder( vec3::EX*r0,  vec3::EX*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder(-vec3::EY*r0, -vec3::EY*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder( vec3::EY*r0,  vec3::EY*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder(-vec3::EZ*r0, -vec3::EZ*r1, r, 1, 8));
	geo_show.add(new GeometryCylinder( vec3::EZ*r0,  vec3::EZ*r1, r, 1, 8));

	for (auto g: geo_show){
		auto *vb = new VertexBuffer("3f,3f,2f");
		g->build(vb);
		buf.add(vb);
	}

	material = create_material(multi_view->resource_manager, Gray, 0.9f, 0, Gray);

	reset();
}

ActionController::~ActionController() {
	delete_geo();
}

#if 0
void ActionController::start_action(Window *_win, const vec3 &_m, Constraint _constraints) {
	if (cur_action)
		end_action(false);
	if (!multi_view->allow_mouse_actions)
		return;
	if (action.name == "")
		return;

	mat = mat4::ID;
	active_win = _win;
	dv = dvp = vec3::ZERO;
	m0 = _m;
	pos0 = pos;
	constraints = _constraints;
	if (constraints == Constraint::FREE)
		pos0 = m0;
	cur_action = ActionMultiViewFactory(action.name, data);
	cur_action->execute_logged(data);
	multi_view->out_action_start();

	MouseWrapper::start(multi_view->session->win);
}


vec3 ActionController::transform_ang(Window *w, const vec3 &ang) {
	auto qmv =  w->local_ang;
	auto qang = quaternion::rotation_v( ang);
	auto q = qmv * qang * qmv.bar();
	return q.get_angles();
}
#endif

vec3 ActionController::project_trans(Constraint mode, const vec3 &v) {
	vec3 r = v;
	if (mode == Constraint::X or mode == Constraint::NEG_X)
		r.y = r.z = 0;
	else if (mode == Constraint::Y or mode == Constraint::NEG_Y)
		r.x = r.z = 0;
	else if (mode == Constraint::Z or mode == Constraint::NEG_Z)
		r.x = r.y = 0;
	else if (mode == Constraint::XY)
		r.z = 0;
	else if (mode == Constraint::XZ)
		r.y = 0;
	else if (mode == Constraint::YZ)
		r.x = 0;
	return r;
}

vec3 ActionController::mirror(Constraint mode) {
	if (mode == Constraint::X or mode == Constraint::NEG_X)
		return vec3::EX;
	else if (mode == Constraint::Y or mode == Constraint::NEG_Y)
		return vec3::EY;
	else if (mode == Constraint::Z or mode == Constraint::NEG_Z)
		return vec3::EZ;
	else if (mode == Constraint::XY)
		return vec3::EZ;
	else if (mode == Constraint::XZ)
		return vec3::EY;
	return vec3::EX;
}

bool cons_neg(ActionController::Constraint c) {
	if (c == ActionController::Constraint::NEG_X)
		return true;
	if (c == ActionController::Constraint::NEG_Y)
		return true;
	if (c == ActionController::Constraint::NEG_Z)
		return true;
	return false;
}

void ActionController::update_action() {
	if (!cur_action)
		return;
#if 0
	//MouseWrapper::update(multi_view);

	vec3 dir = active_win->get_direction();
	vec3 _param = v_0;

	dvp += {multi_view->v,0};
	dv += active_win->unproject(m0 + vec3(multi_view->v,0), m0) - active_win->unproject(m0, m0);

	if (action.mode == ACTION_MOVE) {
		_param = project_trans(constraints, dv);
		_param = multi_view->maybe_snap_v(_param);
	} else if (action.mode == ACTION_ROTATE) {
		//_param = project_trans(constraints, v2 - v1) * 0.003f * multi_view->active_win->zoom();
		_param = project_trans(constraints, dv ^ dir) * 0.003f * multi_view->active_win->zoom();
		if (constraints == Constraint::FREE)
			_param = transform_ang(active_win, vec3(-dvp.y, -dvp.x, 0) * 0.003f);
		_param = multi_view->maybe_snap_v2(_param, pi / 180.0);
	} else if (action.mode == ACTION_SCALE) {
		float sign = cons_neg(constraints) ? -1 : 1;
		_param = vec3(1, 1, 1) + project_trans(constraints, sign * dv) * 0.01f * multi_view->active_win->zoom();
		if (constraints == Constraint::FREE)
			_param = vec3(1, 1, 1) * (1 + dvp.x * 0.01f);
		_param = multi_view->maybe_snap_v2(_param, 0.01f);
	} else if (action.mode == ACTION_MIRROR) {
		_param = mirror(constraints);
		if (constraints == Constraint::FREE)
			_param = active_win->cam->ang * vec3::EX;
	} else {
		param = v_0;
	}
	update_param(_param);
#endif
}

void ActionController::update_param(const vec3 &_param) {
	if (!cur_action)
		return;

	auto m_dt = mat4::translation(pos0);
	auto m_dti = mat4::translation(-pos0);

	param = _param;
	if (action.mode == ACTION_MOVE) {
		mat = mat4::translation(param);
	} else if (action.mode == ACTION_ROTATE) {
		mat = mat4::rotation(param);
		mat = m_dt * mat * m_dti;
	} else if (action.mode == ACTION_SCALE) {
		mat = mat4::scale(param.x, param.y, param.z);
		mat = m_dt * mat * m_dti;
	} else if (action.mode == ACTION_MIRROR) {
		plane pl = plane::from_point_normal(v_0, param);
		mat = mat4::reflection(pl);
		mat = m_dt * mat * m_dti;
	} else {
		mat = mat4::ID;
	}
	cur_action->update_and_notify(data, mat);

	update();

#if 0
	multi_view->out_action_update();
#endif
}



void ActionController::end_action(bool set) {
#if 0
	if (!cur_action)
		return;
	if (set) {
		cur_action->undo(data);
		data->execute(cur_action);
		multi_view->out_action_execute();
	} else {
		cur_action->abort_and_notify(data);
		delete(cur_action);
		multi_view->out_action_abort();
	}
	cur_action = nullptr;
	mat = mat4::ID;
	MouseWrapper::stop(multi_view->session->win);
#endif
}

bool ActionController::is_selecting() {
#if 0
	if (action.mode == ACTION_SELECT)
		return true;
	if (!action.locked)
		return !multi_view->hover_selected();
#endif
	return false;
}

void ActionController::reset() {
	visible = false;
	constraints = Constraint::FREE;
	hover_constraint = Constraint::UNDEFINED;
}

void ActionController::delete_geo() {
	for (Geometry *g: geo)
		delete g;
	geo.clear();
	for (Geometry *g: geo_show)
		delete g;
	geo_show.clear();
}

void ActionController::update() {
	float f = multi_view->view_port.radius * 0.15f;
	//if (multi_view->whole_window)
	f /= 2;

	if (cur_action) {
		pos = pos0;
	} else {
		//pos = multi_view->get_selection_center();
		if (multi_view->selection_box) {
			visible = true;
			pos = multi_view->selection_box->center();
			float box_size = multi_view->selection_box->size().length();
			f = clamp(box_size * 0.5f, f, f*3);
		} else {
			visible = false;
		}
	}
	auto s = mat4::scale(f, f, f);
	auto t = mat4::translation(pos);
	geo_mat = t * s;

	//multi_view->force_redraw();
}

void ActionController::show(bool show) {
	visible = show;
	update();
}

string ActionController::constraint_name(Constraint c) {
	if (c == Constraint::X or c == Constraint::NEG_X)
		return "x-axis";
	if (c == Constraint::Y or c == Constraint::NEG_Y)
		return "y-axis";
	if (c == Constraint::Z or c == Constraint::NEG_Z)
		return "z-axis";
	if (c == Constraint::XY)
		return "x-y-plane";
	if (c == Constraint::XZ)
		return "x-z-plane";
	if (c == Constraint::YZ)
		return "y-z-plane";
	return "free";
}

string ActionController::action_name(int a) {
	if (a == ACTION_MOVE)
		return "move";
	if ((a == ACTION_ROTATE) or (a == ACTION_ROTATE_2D))
		return "rotate";
	if ((a == ACTION_SCALE) or (a == ACTION_SCALE_2D))
		return "scale";
	if (a == ACTION_MIRROR)
		return "reflect";
	return "???";
}

const ActionController::ACGeoConfig ActionController::ac_geo_config[] = {
	{color(1, 0.8f, 0.8f, 0.7f),Constraint::XY,0},
	{color(1, 0.8f, 0.7f, 0.8f),Constraint::XZ,0},
	{color(1, 0.7f, 0.8f, 0.8f),Constraint::YZ,0},
	{color(1, 0.8f, 0.5f, 0.5f),Constraint::NEG_X,1},
	{color(1, 0.8f, 0.5f, 0.5f),Constraint::X,1},
	{color(1, 0.5f, 0.8f, 0.5f),Constraint::NEG_Y,1},
	{color(1, 0.5f, 0.8f, 0.5f),Constraint::Y,1},
	{color(1, 0.5f, 0.5f, 0.8f),Constraint::NEG_Z,1},
	{color(1, 0.5f, 0.5f, 0.8f),Constraint::Z,1},
	{color(1, 0.8f, 0.8f, 0.8f),Constraint::FREE,2}
};

bool ActionController::geo_allow(int i, const mat4& proj, const mat4& geo_mat) {
	auto c = ac_geo_config[i].constraint;
	vec3 pp = proj.project(geo_mat * v_0);
	vec3 ppx = proj.project(geo_mat * vec3::EX);
	ppx.z = pp.z;
	vec3 ppy = proj.project(geo_mat * vec3::EY);
	ppy.z = pp.z;
	vec3 ppz = proj.project(geo_mat * vec3::EZ);
	ppz.z = pp.z;

	if (c == Constraint::X or c == Constraint::NEG_X)
		return (ppx - pp).length() > 8;
	if (c == Constraint::Y or c == Constraint::NEG_Y)
		return (ppy - pp).length() > 8;
	if (c == Constraint::Z or c == Constraint::NEG_Z)
		return (ppz - pp).length() > 8;
	if (c == Constraint::YZ)
		return ((ppy - pp) ^ (ppz - pp)).length() > 300;
	if (c == Constraint::XZ)
		return ((ppx - pp) ^ (ppz - pp)).length() > 300;
	if (c == Constraint::XY)
		return ((ppx - pp) ^ (ppy - pp)).length() > 300;
	return true;
}

// in 2d mode!
void ActionController::draw(const RenderParams& params, RenderViewData& rvd) {
	//if (!multi_view->allow_mouse_actions)
	//	return;
	if (!visible)
		return;


	for (auto vb: buf) {
		//draw_mesh(params, geo_mat, vb, material);
		auto shader = rvd.get_shader(material, 0, "default", "");
		auto& rd = rvd.start(params, geo_mat, shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
		rd.apply(params);
		params.command_buffer->draw(vb);
	}

#if HAS_LIB_GL
	nix::set_z(false, false);
	mat4 m = mat * geo_mat;
	nix::set_model_matrix(m);
	nix::bind_texture(0, nullptr);
	nix::set_shader(win->ctx->default_3d.get());
	win->set_projection_matrix();
	foreachi(Geometry *g, geo_show, i) {
		if (!geo_allow(i, win, m))
			continue;
		if (ac_geo_config[i].constraint == hover_constraint)
			nix::set_material(White, 1, 0, White);
		else
			nix::set_material(Black, 1, 0, ac_geo_config[i].col);
		nix::draw_triangles(buf[i]);
	}
	nix::set_z(false, false);
	nix::disable_alpha();
	nix::set_model_matrix(mat4::ID);
	win->set_projection_matrix_pixel();

	if (in_use()) {
		win->drawing_helper->set_color(color(1, 0.2f, 0.7f, 0.2f));
		win->drawing_helper->set_line_width(2.0f);
		float r = multi_view->cam.radius * 10;
		if (constraints == Constraint::X or constraints == Constraint::NEG_X)
			win->drawing_helper->draw_line(pos - vec3::EX * r, pos + vec3::EX * r);
		if (constraints == Constraint::Y or constraints == Constraint::NEG_Y)
			win->drawing_helper->draw_line(pos - vec3::EY * r, pos + vec3::EY * r);
		if (constraints == Constraint::Z or constraints == Constraint::NEG_Z)
			win->drawing_helper->draw_line(pos - vec3::EZ * r, pos + vec3::EZ * r);
	}

	nix::set_shader(win->ctx->default_2d.get());

	if (win == multi_view->mouse_win) {
		vec3 pp = win->project(pos);

		if ((hover_constraint != Constraint::UNDEFINED) and !in_use()) {
			win->drawing_helper->set_color(scheme.TEXT);
			win->drawing_helper->draw_str(pp.x + 80, pp.y + 40, action_name(action.mode) + ": " + constraint_name(hover_constraint));
		}
	}

	if (in_use() and (win == multi_view->active_win)) {
		vec3 pp = win->project(pos);

		float x0 = pp.x + 120;//multi_view->m.x + 100;//win->dest.x1 + 120;
		float y0 = pp.y + 40;//multi_view->m.y + 50;//win->dest.y1 + 100;

		string s;
		if (action.mode == ACTION_MOVE) {
			vec3 t = param;
			string unit = multi_view->get_unit_by_zoom(t);
			s = f2s(t.x, 2) + " " + unit + "\n" + f2s(t.y, 2) + " " + unit;
			if (multi_view->mode3d)
				s += "\n" + f2s(t.z, 2) + " " + unit;
		} else if ((action.mode == ACTION_ROTATE) or (action.mode == ACTION_ROTATE_2D)) {
			vec3 r = param * 180.0f / pi;
			s = format("%.1f°\n%.1f°\n%.1f°", r.x, r.y, r.z);
		} else if ((action.mode == ACTION_SCALE) or (action.mode == ACTION_SCALE_2D)) {
			if (multi_view->mode3d)
				s = format("%.1f%%\n%.1f%%\n%.1f%%", param.x*100, param.y*100, param.z*100);
			else
				s = format("%.1f%%\n%.1f%%", param.x*100, param.y*100);
		}
		win->drawing_helper->set_color(scheme.TEXT);
		win->drawing_helper->draw_str(x0, y0, s, TextAlign::RIGHT);
	}
#endif
}

void ActionController::draw_post() {
}

ActionController::Constraint ActionController::get_hover(vec3 &tp) {
	if (!visible)
		return Constraint::UNDEFINED;
	float z_min = 1;
	int priority = -1;
	auto hover = Constraint::UNDEFINED;
#if 0
	foreachi(Geometry *g, geo, i) {
		vec3 t;
		if (!geo_allow(i, multi_view->mouse_win, geo_mat))
			continue;
		if (g->is_mouse_over(multi_view->mouse_win, geo_mat, t)) {
			float z = multi_view->mouse_win->project(t).z;
			if ((z < z_min) or (ac_geo_config[i].priority >= priority)) {
				hover = ac_geo_config[i].constraint;
				priority = ac_geo_config[i].priority;
				z_min = z;
				tp = t;
			}
		}
	}
	hover_constraint = hover;
#endif
	return hover;
}

bool ActionController::on_left_button_down() {
	if (!visible and action.locked)
		return false;
#if 0
	vec3 hp = multi_view->hover.point;
	hover_constraint = get_hover(hp);
	if (hover_constraint != Constraint::UNDEFINED) {
		start_action(multi_view->active_win, hp, hover_constraint);
		return true;
	}
	if (multi_view->hover.index >= 0){
		start_action(multi_view->active_win, hp, Constraint::FREE);
		return true;
	}
#endif
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

