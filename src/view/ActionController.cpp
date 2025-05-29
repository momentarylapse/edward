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
#include <y/renderer/scene/RenderViewData.h>
#include <lib/base/iter.h>
#include <lib/math/mat3.h>
#include <lib/math/plane.h>
#include <lib/os/msg.h>

#include "DrawingHelper.h"

const float ActionController::PIXEL_RADIUS = 200;

#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ActionController::Manipulator::Manipulator(MultiView* multi_view) {
	scale = 1;
	pos = v_0;
}

ActionController::ActionController(MultiView *view) :
		in_view_changed(this, &ActionController::update_manipulator), // FIXME this uses some view matrices that will only be updated later during drawing...
		in_selection_changed(this, [this] {
			if (multi_view->selection_box)
				visible = allowed;
			else
				visible = false;
			update_manipulator();
			out_changed();
		}),
		manipulator(view)
{
	multi_view = view;
	mat = mat4::ID;
	action.mode = MouseActionMode::MOVE;

	//reset();
}

ActionController::~ActionController() = default;

void ActionController::start_action(Data* _data, ActionMultiView* a, const vec3 &_m, Constraint _constraints) {
	if (cur_action)
		end_action(false);
	if (!allowed)
		return;;
	//if (action.name == "")
	//	return;

	data = _data;
	mat = mat4::ID;
	param = vec3(0, 0, 0);
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
	dv += active_win->unproject(m0.xy() + d, m0) - active_win->unproject(m0.xy(), m0);

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

string ActionController::action_name() const {
	return action.name();
}


MouseActionMode ActionController::action_mode() const {
	return action.mode;
}

void ActionController::set_action_mode(MouseActionMode mode) {
	action.mode = mode;
}

void ActionController::set_allowed(bool _allowed) {
	allowed = _allowed;
	visible = allowed and multi_view->selection_box;
	update_manipulator();
}

void ActionController::Manipulator::update(ActionController* ac) {
	if (ac->cur_action) {
		pos = pos0;
	} else {
		scale = ac->multi_view->view_port.radius * 0.15f;
		//if (multi_view->whole_window)
		scale /= 2;
		//pos = multi_view->get_selection_center();
		if (ac->multi_view->selection_box) {
			pos = ac->multi_view->selection_box->center();
			pos0 = pos;
			float box_size = ac->multi_view->selection_box->size().length();
			scale = clamp(box_size * 0.5f, scale, scale*3);
		}

		auto L = ac->multi_view->active_window->linear_projection(pos);
		//auto Linv = L.inverse();
		scale = PIXEL_RADIUS / sqrtf((L * vec3::EX).length_sqr() + (L * vec3::EY).length_sqr() + (L * vec3::EZ).length_sqr());
	}
	auto s = mat4::scale(scale, scale, scale);
	auto t = mat4::translation(pos);
	//geo_mat = ac->mat * t * s;

	//multi_view->force_redraw();
}

void ActionController::update_manipulator() {
	manipulator.update(this);
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
	{color(1, 0.8f, 0.8f, 0.4f),Constraint::XY,0},
	{color(1, 0.8f, 0.4f, 0.8f),Constraint::XZ,0},
	{color(1, 0.4f, 0.8f, 0.8f),Constraint::YZ,0},
	{color(1, 1, 0.2f, 0.2f),Constraint::NEG_X,1},
	{color(1, 1, 0.2f, 0.2f),Constraint::X,1},
	{color(1, 0.2f, 1, 0.2f),Constraint::NEG_Y,1},
	{color(1, 0.2f, 1, 0.2f),Constraint::Y,1},
	{color(1, 0.2f, 0.2f, 1),Constraint::NEG_Z,1},
	{color(1, 0.2f, 0.2f, 1),Constraint::Z,1},
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
void ActionController::__draw(const RenderParams& params, RenderViewData& rvd) {
	//if (!multi_view->allow_mouse_actions)
	//	return;
	if (!visible)
		return;

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

void ActionController::draw_manipulator_default(Painter* p) {
	if (auto w = multi_view->hover_window) {
		p->set_line_width(DrawingHelper::LINE_THIN);
		//p->set_fill(false);
		const float r = manipulator.scale;
		constexpr float rr = 8;
		const vec3 p0 = manipulator.pos;
		Array basis = {vec3::EX, vec3::EY, vec3::EZ};
		Array colors = {color(1, 1, 0.2f, 0.2f), color(1, 0.2f, 1, 0.2f), color(1, 0.2f, 0.2f, 1)};

		p->set_color(White.with_alpha(0.7f));
		int N=64;
		{
			Array<vec2> points;
			for (int i=0; i<N; i++) {
				float phi = (float)i / (float)N * pi/2;
				points.add(w->project(p0 + vec3(cosf(phi), sinf(phi), 0) * r * 0.8f).xy());
			}
			p->draw_lines(points);
		}
		{
			Array<vec2> points;
			for (int i=0; i<N; i++) {
				float phi = (float)i / (float)N * pi/2;
				points.add(w->project(p0 + vec3(cosf(phi), 0, sinf(phi)) * r * 0.8f).xy());
			}
			p->draw_lines(points);
		}
		{
			Array<vec2> points;
			for (int i=0; i<N; i++) {
				float phi = (float)i / (float)N * pi/2;
				points.add(w->project(p0 + vec3(0, cosf(phi), sinf(phi)) * r * 0.8f).xy());
			}
			p->draw_lines(points);
		}


		manipulator.handle_positions.clear();

		Array c0 = {Constraint::NEG_X, Constraint::NEG_Y, Constraint::NEG_Z};
		Array c1 = {Constraint::X, Constraint::Y, Constraint::Z};

		for (const auto& [i, e]: enumerate(basis)) {
			vec3 a = p0 - r * e;
			vec3 b = p0 + r * e;
			p->set_color(White.with_alpha(0.7f));
			p->draw_line(w->project(a).xy(), w->project(b).xy());

			if (multi_view->hover and multi_view->hover->type == MultiViewType::ACTION_MANAGER and multi_view->hover->index == (int)c0[i])
				p->set_color(colors[i]);
			else
				p->set_color(color::interpolate(colors[i], White, 0.5f));
			p->draw_circle(w->project(a).xy(), rr);

			if (multi_view->hover and multi_view->hover->type == MultiViewType::ACTION_MANAGER and multi_view->hover->index == (int)c1[i])
				p->set_color(colors[i]);
			else
				p->set_color(color::interpolate(colors[i], White, 0.5f));
			p->draw_circle(w->project(b).xy(), rr);

			manipulator.handle_positions.add(a);
			manipulator.handle_positions.add(b);
		}
		if (multi_view->hover and multi_view->hover->type == MultiViewType::ACTION_MANAGER and multi_view->hover->index == (int)Constraint::FREE)
			p->set_color(Gray);
		else
			p->set_color(White);
		manipulator.handle_positions.add(p0);
		p->draw_circle(w->project(p0).xy(), rr*2);
		//p->set_fill(true);
	}
}

void ActionController::draw_action_stats(Painter* p) {
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
			s = format("%.1f %%\n%.1f %%\n%.1f %%", param.x*100, param.y*100, param.z*100);
		else
			s = format("%.1f%%\n%.1f%%", param.x*100, param.y*100);
	}
	drawing2d::draw_boxed_str(p, {100, 100}, s);
}

void ActionController::draw_manipulator_active(Painter* p) {
	auto w = multi_view->hover_window;
	const vec3 p0 = manipulator.pos;
	Array basis = {vec3::EX, vec3::EY, vec3::EZ};
	Array colors = {color(1, 1, 0.2f, 0.2f), color(1, 0.2f, 1, 0.2f), color(1, 0.2f, 0.2f, 1)};
	const float r = manipulator.scale;
	constexpr float rr = 8;

	float sign = 1.0f;
	if (constraints == Constraint::NEG_X or constraints == Constraint::NEG_Y or constraints == Constraint::NEG_Z)
		sign = -1.0f;
	int axis = -1;
	if (constraints == Constraint::X or constraints == Constraint::NEG_X)
		axis = 0;
	if (constraints == Constraint::Y or constraints == Constraint::NEG_Y)
		axis = 1;
	if (constraints == Constraint::Z or constraints == Constraint::NEG_Z)
		axis = 2;

	// "infinite" constraint axis
	if (constraints != Constraint::FREE) {
		p->set_color(colors[axis]);
		p->set_line_width(DrawingHelper::LINE_MEDIUM);
		vec2 a = w->project(p0).xy();
		vec2 b = w->project(p0 + w->pixel_to_size(2) * basis[axis]).xy();
		vec2 dir = (b - a).normalized();
		p->draw_line(a - dir * 4000, a + dir * 4000);
	}

	if (action.mode == MouseActionMode::MOVE) {
		vec3 pos = p0 + param;

		if (constraints == Constraint::FREE) {
			p->set_color(White);
			p->draw_circle(w->project(pos).xy(), rr*2);
		}
		if (constraints != Constraint::FREE) {
			p->set_color(colors[axis]);
			p->draw_circle(w->project(pos + sign * r * basis[axis]).xy(), rr);
		}
	}
}



void ActionController::draw_post(Painter* p) {
	if (!visible)
		return;

	if (performing_action()) {
		draw_manipulator_active(p);

		draw_action_stats(p);
	} else {
		draw_manipulator_default(p);
	}
}

ActionController::Constraint ActionController::get_hover(MultiViewWindow* win, const vec2& m, vec3 &tp) const {
	if (!visible)
		return Constraint::UNDEFINED;
	float z_min = 1;
	int priority = -1;
	auto hover = Constraint::UNDEFINED;

	for (const auto& [i, hp]: enumerate(manipulator.handle_positions)) {
		const auto pp = win->project(hp);
		if (pp.z > 0 and pp.z < z_min and (pp.xy() - m).length() < 20) {
			if (i == 0)
				hover = Constraint::NEG_X;
			if (i == 1)
				hover = Constraint::X;
			if (i == 2)
				hover = Constraint::NEG_Y;
			if (i == 3)
				hover = Constraint::Y;
			if (i == 4)
				hover = Constraint::NEG_Z;
			if (i == 5)
				hover = Constraint::Z;
			if (i == 6)
				hover = Constraint::FREE;
			tp = hp;
			z_min = pp.z;
		}
	}

	/*for (const auto& [i, g]: enumerate(manipulator.geo)) {
		vec3 t;
		int index;
		if (!geo_allow(i, win, manipulator.geo_mat))
			continue;
		if (g->is_mouse_over(win, manipulator.geo_mat, m, t, index, true)) {
			float z = win->project(t).z;
			if ((z < z_min) or (geo_config[i].priority >= priority)) {
				hover = geo_config[i].constraint;
				priority = geo_config[i].priority;
				z_min = z;
				tp = t;
			}
		}
	}*/
	//hover_constraint = hover;
	return hover;
}

bool ActionController::performing_action() {
	return cur_action;
}

bool ActionController::on_mouse_move(const vec2& m, const vec2& d) {
	if (performing_action()) {
		update_action(d);
		return true;
	}
	return false;
}

