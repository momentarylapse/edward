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
#include <data/mesh/PolygonMesh.h>
#include <data/mesh/GeometryBall.h>
#include <data/mesh/GeometryCylinder.h>
#include <data/mesh/GeometryTorus.h>
#include <data/mesh/GeometryCube.h>
#include <y/graphics-impl.h>
#include <y/renderer/world/geometry/RenderViewData.h>
#include <lib/base/iter.h>
#include <lib/math/plane.h>
#include <lib/os/msg.h>

#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ActionController::Manipulator::Manipulator(MultiView* multi_view) {
	geo_mat = mat4::ID;
	scale = 1;
	pos = v_0;

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
	geo_show.add(new GeometryTorus(v_0, vec3::EZ, 1.0f, r, 128, 8));
	geo_show.add(new GeometryTorus(v_0, vec3::EY, 1.0f, r, 128, 8));
	geo_show.add(new GeometryTorus(v_0, vec3::EX, 1.0f, r, 128, 8));
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

	for (int i=0; i<geo.num; i++) {
		auto m = create_material(multi_view->resource_manager, Black, 0.9f, 0, geo_config[i].col);
		m->pass0.z_buffer = false;
		m->pass0.z_test = false;
		materials.add(m);
	}
	material_hover = create_material(multi_view->resource_manager, Black, 0.9f, 0, White);
	material_hover->pass0.z_buffer = false;
	material_hover->pass0.z_test = false;
}

ActionController::ActionController(MultiView *view) : manipulator(view) {
	multi_view = view;
	mat = mat4::ID;
	action.mode = MouseActionMode::MOVE;

	//reset();
}

ActionController::~ActionController() = default;

void ActionController::start_action(ActionMultiView* a, const vec3 &_m, Constraint _constraints) {
	if (cur_action)
		end_action(false);
//	if (!multi_view->allow_mouse_actions)
//		return;
	//if (action.name == "")
	//	return;

	mat = mat4::ID;
	//active_win = multi_view->active_window;
	dv = dvp = vec3::ZERO;
	m0 = _m;
	manipulator.pos0 = manipulator.pos;
	constraints = _constraints;
	if (constraints == Constraint::FREE)
		manipulator.pos0 = m0;
	cur_action = a;
	cur_action->execute_logged(data);
	//multi_view->out_action_start();

	//MouseWrapper::start(multi_view->session->win);
}


vec3 ActionController::transform_ang(MultiViewWindow* w, const vec3& ang) {
	auto qmv =  w->local_ang;
	auto qang = quaternion::rotation_v(ang);
	auto q = qmv * qang * qmv.bar();
	return q.get_angles();
}

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

void ActionController::update_action(const vec2& d) {
	if (!cur_action)
		return;

	//MouseWrapper::update(multi_view);
	auto active_win = multi_view->active_window;

	vec3 dir = active_win->direction();
	vec3 _param = v_0;

	dvp += {d,0};
	dv += active_win->unproject(m0 + vec3(d,0), m0) - active_win->unproject(m0, m0);

	if (action.mode == MouseActionMode::MOVE) {
		_param = project_trans(constraints, dv);
		_param = multi_view->maybe_snap_v(_param);
	} else if (action.mode == MouseActionMode::ROTATE) {
		//_param = project_trans(constraints, v2 - v1) * 0.003f * multi_view->active_win->zoom();
		_param = project_trans(constraints, dv ^ dir) * 0.003f * active_win->zoom();
		if (constraints == Constraint::FREE)
			_param = transform_ang(active_win, vec3(-dvp.y, -dvp.x, 0) * 0.003f);
		_param = multi_view->maybe_snap_v2(_param, pi / 180.0f);
	} else if (action.mode == MouseActionMode::SCALE) {
		float sign = cons_neg(constraints) ? -1 : 1;
		_param = vec3(1, 1, 1) + project_trans(constraints, sign * dv) * 0.01f * active_win->zoom();
		if (constraints == Constraint::FREE)
			_param = vec3(1, 1, 1) * (1 + dvp.x * 0.01f);
		_param = multi_view->maybe_snap_v2(_param, 0.01f);
	} else if (action.mode == MouseActionMode::MIRROR) {
		_param = mirror(constraints);
		if (constraints == Constraint::FREE)
			_param = multi_view->view_port.ang * vec3::EX;
	} else {
		param = v_0;
	}
	update_param(_param);
}

void ActionController::update_param(const vec3 &_param) {
	if (!cur_action)
		return;

	auto m_dt = mat4::translation(manipulator.pos0);
	auto m_dti = mat4::translation(-manipulator.pos0);

	param = _param;
	if (action.mode == MouseActionMode::MOVE) {
		mat = mat4::translation(param);
	} else if (action.mode == MouseActionMode::ROTATE) {
		mat = mat4::rotation(param);
		mat = m_dt * mat * m_dti;
	} else if (action.mode == MouseActionMode::SCALE) {
		mat = mat4::scale(param.x, param.y, param.z);
		mat = m_dt * mat * m_dti;
	} else if (action.mode == MouseActionMode::MIRROR) {
		plane pl = plane::from_point_normal(v_0, param);
		mat = mat4::reflection(pl);
		mat = m_dt * mat * m_dti;
	} else {
		mat = mat4::ID;
	}
	cur_action->update_and_notify(data, mat);

	update_manipulator();

#if 0
	multi_view->out_action_update();
#endif
}



void ActionController::end_action(bool set) {
	if (!cur_action)
		return;
	if (set) {
		cur_action->undo(data);
		data->execute(cur_action);
	//	multi_view->out_action_execute();
	} else {
		cur_action->abort_and_notify(data);
		delete(cur_action);
	//	multi_view->out_action_abort();
	}
	cur_action = nullptr;
	mat = mat4::ID;
	//MouseWrapper::stop(multi_view->session->win);
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

/*void ActionController::reset() {
	msg_write("reset");
	visible = false;
	constraints = Constraint::FREE;
	hover_constraint = Constraint::UNDEFINED;
}*/

void ActionController::Manipulator::update(ActionController* ac) {
	if (ac->cur_action) {
		pos = pos0;
	} else {
		scale = ac->multi_view->view_port.radius * 0.15f;
		//if (multi_view->whole_window)
		scale /= 2;
		//pos = multi_view->get_selection_center();
		if (ac->multi_view->selection_box) {
			ac->visible = true;
			pos = ac->multi_view->selection_box->center();
			pos0 = pos;
			float box_size = ac->multi_view->selection_box->size().length();
			scale = clamp(box_size * 0.5f, scale, scale*3);
		} else {
			ac->visible = false;
		}
	}
	auto s = mat4::scale(scale, scale, scale);
	auto t = mat4::translation(pos);
	geo_mat = ac->mat * t * s;

	//multi_view->force_redraw();
}

void ActionController::update_manipulator() {
	manipulator.update(this);
}


void ActionController::show(bool show) {
	visible = show;
	update_manipulator();
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

string MouseAction::name() const {
	if (mode == MouseActionMode::MOVE)
		return "move";
	if ((mode == MouseActionMode::ROTATE) or (mode == MouseActionMode::ROTATE_2D))
		return "rotate";
	if ((mode == MouseActionMode::SCALE) or (mode == MouseActionMode::SCALE_2D))
		return "scale";
	if (mode == MouseActionMode::MIRROR)
		return "reflect";
	return "???";
}

const ActionController::GeoConfig ActionController::geo_config[] = {
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

bool ActionController::geo_allow(int i, MultiViewWindow* win, const mat4& geo_mat) {
	auto c = geo_config[i].constraint;
	vec3 pp = win->project(geo_mat * v_0);
	vec3 ppx = win->project(geo_mat * vec3::EX);
	ppx.z = pp.z;
	vec3 ppy = win->project(geo_mat * vec3::EY);
	ppy.z = pp.z;
	vec3 ppz = win->project(geo_mat * vec3::EZ);
	ppz.z = pp.z;

	if (c == Constraint::X or c == Constraint::NEG_X)
		return (ppx - pp).length() > 8;
	if (c == Constraint::Y or c == Constraint::NEG_Y)
		return (ppy - pp).length() > 8;
	if (c == Constraint::Z or c == Constraint::NEG_Z)
		return (ppz - pp).length() > 8;
	if (c == Constraint::YZ)
		return vec3::cross(ppy - pp, ppz - pp).length() > 300;
	if (c == Constraint::XZ)
		return vec3::cross(ppx - pp, ppz - pp).length() > 300;
	if (c == Constraint::XY)
		return vec3::cross(ppx - pp, ppy - pp).length() > 300;
	return true;
}

// in 2d mode!
void ActionController::draw(const RenderParams& params, RenderViewData& rvd) {
	//if (!multi_view->allow_mouse_actions)
	//	return;
	if (!visible)
		return;

	for (const auto& [i, vb]: enumerate(manipulator.buf)) {
		if (!geo_allow(i, multi_view->active_window, manipulator.geo_mat))
			continue;
		//draw_mesh(params, geo_mat, vb, material);
		auto m = manipulator.materials[i];
		if (multi_view->hover and multi_view->hover->type == MultiViewType::ACTION_MANAGER and multi_view->hover->index == (int)geo_config[i].constraint)
			m = manipulator.material_hover.get();
		auto shader = rvd.get_shader(m, 0, "default", "");
		auto& rd = rvd.start(params, manipulator.geo_mat, shader, *m, 0, PrimitiveTopology::TRIANGLES, vb);
		rd.apply(params);
#ifdef USING_VULKAN
		params.command_buffer->draw(vb);
#else
		nix::draw_triangles(vb);
#endif
	}

#if 0 //HAS_LIB_GL
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

void ActionController::draw_post(Painter* p) {
	if (!visible)
		return;
	Array<vec2> lines;

#if 0
	auto pr = [this] (const vec3& v) {
		return multi_view->projection.project(geo_mat * v).xy();
	};

	p->set_line_width(10);
	for (float phi=0; phi<2*pi; phi+=0.10f)
		lines.add(pr({cos(phi), 0, sin(phi)}));
	p->set_color(color(1, 1, 0, 1));
	p->draw_lines(lines);

	lines.clear();
	for (float phi=0; phi<2*pi; phi+=0.10f)
		lines.add(pr({cos(phi), sin(phi), 0}));
	p->set_color(color(1, 1, 1, 0));
	p->draw_lines(lines);

	lines.clear();
	for (float phi=0; phi<2*pi; phi+=0.10f)
		lines.add(pr({0, cos(phi), sin(phi)}));
	p->set_color(color(1, 0, 1, 1));
	p->draw_lines(lines);

	float r1 = 1.3f;
	float r2 = 0.7f;
	p->set_color(Red);
	p->draw_line(pr({-r1, 0, 0}), pr({-r2, 0, 0}));
	p->draw_line(pr({r1, 0, 0}), pr({r2, 0, 0}));
	p->set_color(Green);
	p->draw_line(pr({0, -r1, 0}), pr({0, -r2, 0}));
	p->draw_line(pr({0, r1, 0}), pr({0, r2, 0}));
	p->set_color(Blue);
	p->draw_line(pr({0, 0, -r1}), pr({0, 0, -r2}));
	p->draw_line(pr({0, 0, r1}), pr({0, 0, r2}));
#endif


	if (in_use()) { // and (win == multi_view->active_win)) {
		//vec3 pp = win->project(pos);

		//float x0 = pp.x + 120;//multi_view->m.x + 100;//win->dest.x1 + 120;
		//float y0 = pp.y + 40;//multi_view->m.y + 50;//win->dest.y1 + 100;

		string s;
		if (action.mode == MouseActionMode::MOVE) {
			vec3 t = param;
			string unit = multi_view->get_unit_by_zoom(t);
			s = f2s(t.x, 2) + " " + unit + "\n" + f2s(t.y, 2) + " " + unit;
		//	if (multi_view->mode3d)
				s += "\n" + f2s(t.z, 2) + " " + unit;
		} else if ((action.mode == MouseActionMode::ROTATE) or (action.mode == MouseActionMode::ROTATE_2D)) {
			vec3 r = param * 180.0f / pi;
			s = format("%.1f°\n%.1f°\n%.1f°", r.x, r.y, r.z);
		} else if ((action.mode == MouseActionMode::SCALE) or (action.mode == MouseActionMode::SCALE_2D)) {
			if (true) //multi_view->mode3d)
				s = format("%.1f%%\n%.1f%%\n%.1f%%", param.x*100, param.y*100, param.z*100);
			else
				s = format("%.1f%%\n%.1f%%", param.x*100, param.y*100);
		}
	//	win->drawing_helper->set_color(scheme.TEXT);
	//	win->drawing_helper->draw_str(x0, y0, s, TextAlign::RIGHT);
		p->set_color(Black);
		p->draw_str({100,100}, s);
	}
}

ActionController::Constraint ActionController::get_hover(MultiViewWindow* win, const vec2& m, vec3 &tp) {
	if (!visible)
		return Constraint::UNDEFINED;
	float z_min = 1;
	int priority = -1;
	auto hover = Constraint::UNDEFINED;
	for (const auto& [i, g]: enumerate(manipulator.geo)) {
		vec3 t;
		if (!geo_allow(i, win, manipulator.geo_mat))
			continue;
		if (g->is_mouse_over(win, manipulator.geo_mat, m, t)) {
			float z = win->project(t).z;
			if ((z < z_min) or (geo_config[i].priority >= priority)) {
				hover = geo_config[i].constraint;
				priority = geo_config[i].priority;
				z_min = z;
				tp = t;
			}
		}
	}
	//hover_constraint = hover;
	return hover;
}

#if 0
bool ActionController::on_left_button_down(const vec2& m) {
	if (!visible and action.locked)
		return false;
	vec3 hp; // = multi_view->hover.point;
	hover_constraint = get_hover(multi_view->hover_window, m, hp);
	if (hover_constraint != Constraint::UNDEFINED) {
		//start_action(multi_view->active_win, hp, hover_constraint);
		return true;
	}
	/*if (multi_view->hover.index >= 0){
		start_action(multi_view->active_win, hp, Constraint::FREE);
		return true;
	}*/
	return false;
}

void ActionController::on_left_button_up(const vec2& m) {
	end_action(true);

	update();
}
#endif

bool ActionController::in_use() {
	return cur_action;
}

/*void ActionController::on_mouse_move(const vec2& m, const vec2& d) {
	update_action();
}*/

