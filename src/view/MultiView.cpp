//
// Created by Michael Ankele on 2025-01-20.
//

#include "MultiView.h"
#include "ActionController.h"
#include "DrawingHelper.h"
#include <Session.h>
#include <action/ActionMultiView.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/math/mat3.h>
#include <multiview/SingleData.h>
#include <renderer/path/RenderPath.h>
#include <renderer/world/geometry/GeometryRenderer.h>
#include <renderer/world/pass/ShadowRendererX.h>

#include "EdwardWindow.h"
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/Entity.h>
#include <y/graphics-impl.h>

#include "Mode.h"

extern float global_shadow_box_size;

MultiViewWindow::MultiViewWindow(MultiView* _multi_view) {
	multi_view = _multi_view;
	rvd.scene_view = multi_view->view_port.scene_view.get();
}

vec3 MultiViewWindow::project(const vec3& v) const {
	return to_pixels.project(v);
}

vec3 MultiViewWindow::unproject(const vec3& v, const vec3& zref) const {
	vec3 op = project(zref);
	vec3 r = v;
	r.z = op.z;
	return to_pixels.inverse().project(r);
}

vec3 MultiViewWindow::direction() const {
	return multi_view->view_port.ang * vec3::EZ;
}

float MultiViewWindow::zoom() const {

	//return 1000.0f / radius;
	//if (multi_view->mode3d)
		return area.height() / multi_view->view_port.radius;
	//else
	//	return area.height() * 0.8f / multi_view->view_port.radius;
}

#define GRID_CONST	5.0f

float MultiViewWindow::get_grid_d() const {
	return pow(10.0f, ceil(log10(GRID_CONST / zoom())));
}

int grid_level(int i) {
	if (i == 0)
		return 0;
	if (i % 10 == 0)
		return 1;
	if (i % 100 == 0)
		return 2;
	return 3;
}

const float LOW_MAX = 0.25f;
const float MID_MAX = 0.5f;

float grid_density(int level, float d_err) {
	if (level == 0)
		return 1;
	if (level == 1)
		d_err += 1;
	return min((float)pow(10.0f, d_err-1.0f) * LOW_MAX, MID_MAX);
}


int MultiViewWindow::active_grid() const {
	vec3 d = direction();
	d.x = abs(d.x);
	d.y = abs(d.y);
	d.z = abs(d.z);
	if (d.x > d.y and d.x > d.z)
		return 0;
	if (d.y > d.z)
		return 1;
	return 2;
}

vec3 MultiViewWindow::active_grid_direction() const {
	vec3 dd = vec3::EZ;
	int ag = active_grid();
	if (ag == 0)
		dd = vec3::EX;
	if (ag == 1)
		dd = vec3::EY;

	if (dd * direction() < 0)
		return -dd;
	return dd;
}

mat3 MultiViewWindow::active_grid_frame() const {
	const vec3 dir = active_grid_direction();
	const vec3 up = dir.ortho();
	const vec3 right = dir ^ up;
	return {right, up, dir};
}

mat3 MultiViewWindow::edit_frame() const {
	return active_grid_frame();
}

void MultiViewWindow::prepare(const RenderParams& params) {
	rvd.scene_view->cam->update_matrices(params.desired_aspect_ratio);
	rvd.set_projection_matrix(rvd.scene_view->cam->m_projection);
	rvd.set_view_matrix(rvd.scene_view->cam->m_view);
	rvd.update_lights();
}

void MultiViewWindow::draw(const RenderParams& params) {
	rvd.begin_draw();
	multi_view->session->drawing_helper->set_window(this);

	multi_view->session->cur_mode->on_draw_win(params, this);
}


class MultiViewShadowGeometryEmitter : public GeometryEmitter {
public:
	MultiView* multi_view;
	MultiViewShadowGeometryEmitter(MultiView* mv, SceneView& scene_view) : GeometryEmitter(RenderPathType::Forward, scene_view) {
		multi_view = mv;
	}
	void draw(const RenderParams& params) override {
		cur_rvd.begin_draw();
		if (override_view)
			cur_rvd.set_view_matrix(*override_view);
		else
			cur_rvd.set_view_matrix(scene_view.cam->view_matrix());
		if (override_projection)
			cur_rvd.set_projection_matrix(*override_projection);
		multi_view->session->cur_mode->on_draw_shadow(params, cur_rvd);
	}
};



MultiView::MultiView(Session* s) : obs::Node<Renderer>("multiview"),
		in_data_changed(this, [this] {
			if (!action_controller->in_use()) {
				update_selection_box();
				hover = base::None;
			}
		}),
		view_port(this),
		window(this)
{
	session = s;
	resource_manager = session->resource_manager;
	active_window = &window;
	hover_window = &window;
	action_controller = new ActionController(this);


	light_mode = LightMode::FollowCamera;
	default_light = new Light(White, -1, -1);
	default_light->owner = new Entity;
	default_light->owner->ang = quaternion::ID;
	default_light->enabled = true;
	default_light->allow_shadow = true;;
	default_light->light.harshness = 0.5f;
	lights.add(default_light);

	view_port.out_changed >> create_sink([this] {
		action_controller->update_manipulator();
	});

	shadow_renderer = new ShadowRendererX(view_port.scene_view->cam, [this] (SceneView& scene_view) {
		return new MultiViewShadowGeometryEmitter(this, scene_view);
	});
	view_port.scene_view->shadow_maps.add(shadow_renderer->cascades[0].depth_buffer);
	view_port.scene_view->shadow_maps.add(shadow_renderer->cascades[1].depth_buffer);
	add_child(shadow_renderer.get());
}

MultiView::~MultiView() = default;

void MultiView::set_area(const rect& _area) {
	area = _area;
	area_native = {_area.p00() * session->win->ui_scale, _area.p11() * session->win->ui_scale};
	window.area = area;
	window.area_native = area_native;
}


void MultiView::prepare(const RenderParams& params) {
	view_port.cam->owner->ang = view_port.ang;
	view_port.cam->owner->pos = view_port.pos - view_port.cam->owner->ang * vec3::EZ * view_port.radius;
	view_port.cam->min_depth = view_port.radius * 0.01f;
	view_port.cam->max_depth = view_port.radius * 300;
	view_port.cam->update_matrices(area.width() / area.height());
	global_shadow_box_size = view_port.radius * 8;

	window.local_ang = view_port.ang;
	window.view = view_port.cam->m_view;
	window.projection = view_port.cam->m_projection;

	// 3d -> pixel
	window.to_pixels = mat4::translation({area.x1, area.y1, 0})
		* mat4::scale(area.width()/2, area.height()/2, 1)
		* mat4::translation({1.0f, 1.0f, 0})
		* window.projection * window.view;

	{
		if (light_mode == LightMode::FollowCamera)
			default_light->owner->ang = view_port.ang;
		lights = {default_light};
	}

	session->cur_mode->on_prepare_scene(params);

	view_port.scene_view->lights = lights;
	view_port.scene_view->choose_shadows();

	window.prepare(params);

	for (int i: view_port.scene_view->shadow_indices) {
		shadow_renderer->set_projection(view_port.scene_view->lights[i]->shadow_projection);
		shadow_renderer->render(params);
	}

	//Renderer::prepare(params);
}

void MultiView::draw(const RenderParams& params) {
	//engine.physical_aspect_ratio = pp->native_area.width() / pp->native_area.height();

	window.prepare(params); // why is this neccessary?
	window.draw(params);
}


void MultiView::on_mouse_move(const vec2& m, const vec2& d) {
	if (action_controller->cur_action) {
		action_controller->update_action(d);
		action_controller->visible = true;
		return;
	}
	if (selection_area) {
		selection_area = rect(selection_area->p00(), m);
		select_in_rect(active_window, *selection_area);
	/*	auto s = get_selection(hover_window, *selection_area);
		// TODO shift/control
		if (s != selection) {
			selection = s;
			update_selection_box();
		}*/
		return;
	} else if (session->win->button(0) and possibly_selecting) {
		// start selection rect
		selection_area = rect(m - d, m);
		//update_selection_box();
		return;
	}


	if (!session->win->button(0) and !session->win->button(1) and !session->win->button(2))
		hover = get_hover(hover_window, m);
	//action_controller->on_mouse_move(m, d);
	// TODO if busy... return

	// left -> ...
	if (session->win->button(0) and false)
		view_port.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));

	// right -> rotate
	if (session->win->button(2))
		view_port.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));

	// middle -> move
	if (session->win->button(1))
		view_port.move(vec3(-d.x, d.y, 0) / 800.0f); // / window size?
}

void MultiView::on_mouse_leave() {
	hover = base::None;
}

void MultiView::clear_selection() {
	for (auto& d: data_sets)
		for (int i=0; i<d.array->num; i++)
			reinterpret_cast<multiview::SingleData*>(d.array->simple_element(i))->is_selected = false;
	selection_box = base::None;
	action_controller->visible = false;
	out_selection_changed();
}

void MultiView::select_in_rect(MultiViewWindow* win, const rect& _r) {
	const auto r = _r.canonical();
	if (f_select)
		f_select(win, r);

	update_selection_box();
	out_selection_changed();
}

void MultiView::select_points_in_rect(MultiViewWindow* win, const rect& r, DynamicArray& array) {
	for (int i=0; i<array.num; i++) {
		auto p = reinterpret_cast<multiview::SingleData*>(array.simple_element(i));
		p->is_selected = r.inside(win->project(p->pos).xy());
	}
}


multiview::SingleData* MultiView::get_hover_item() {
	if (!hover)
		return nullptr;
	for (auto& d: data_sets)
		if (d.type == hover->type)
			return reinterpret_cast<multiview::SingleData*>(d.array->simple_element(hover->index));
	return nullptr;
}

void MultiView::update_selection_box() {
	selection_box = base::None;
	if (f_get_selection_box)
		selection_box = f_get_selection_box();
	action_controller->update_manipulator();
}

base::optional<Box> MultiView::points_get_selection_box(const DynamicArray& _array) {
	base::optional<Box> box;
	auto& array = const_cast<DynamicArray&>(_array);
	for (int i=0; i<array.num; i++) {
		auto p = reinterpret_cast<multiview::SingleData*>(array.simple_element(i));
		if (p->is_selected) {
			if (box)
				box = *box or Box{p->pos, p->pos};
			else
				box = Box{p->pos, p->pos};
		}
	}
	return box;
}


void MultiView::on_left_button_down(const vec2& m) {
	hover = get_hover(hover_window, m);
	possibly_selecting = _allow_select;

	if (!_allow_select)
		return;

	//action_controller->on_left_button_down(m);
	if (hover and hover->type == MultiViewType::ACTION_MANAGER) {
		action_controller->data = session->cur_mode->get_data();
		if (f_create_action and _allow_action)
			action_controller->start_action(f_create_action(), hover->tp, (ActionController::Constraint)hover->index);

	} else if (auto p = get_hover_item()) {
		if (session->win->is_key_pressed(xhui::KEY_SHIFT)) {
			// toggle p
			p->is_selected = !p->is_selected;
			update_selection_box();
			out_selection_changed();
		} else if (session->win->is_key_pressed(xhui::KEY_CONTROL)) {
			// add p
			p->is_selected = true;
			update_selection_box();
			out_selection_changed();
		} else {
			// select p exclusively
			clear_selection();
			p->is_selected = true;
			update_selection_box();
			out_selection_changed();
		}
	} else {
		if (!session->win->is_key_pressed(xhui::KEY_SHIFT) and !session->win->is_key_pressed(xhui::KEY_CONTROL))
			clear_selection();
	}
}

void MultiView::on_left_button_up(const vec2& m) {
	//action_controller->on_left_button_up(m);
	if (action_controller->cur_action) {
		action_controller->end_action(true);
		update_selection_box();
		return;
	}
	selection_area = base::None;
	hover = get_hover(hover_window, m);
	possibly_selecting = false;
}



void MultiView::on_mouse_wheel(const vec2& m, const vec2& d) {
	view_port.zoom(exp(d.y * 0.1f));
}

void MultiView::on_key_down(int key) {
	float d = 0.05f;
	if (key == xhui::KEY_UP)
		view_port.move({0, d, 0});
	if (key == xhui::KEY_DOWN)
		view_port.move({0, -d, 0});
	if (key == xhui::KEY_LEFT)
		view_port.move({-d, 0, 0});
	if (key == xhui::KEY_RIGHT)
		view_port.move({d, 0, 0});
}

void MultiView::on_draw(Painter* p) {
	if (selection_area) {
		p->set_color({0.2, 0,0,1});
		p->draw_rect(selection_area->canonical());
		p->set_fill(false);
		p->set_color(Blue);
		p->set_line_width(2);
		p->draw_rect(selection_area->canonical());
		p->set_fill(true);
	}
	action_controller->draw_post(p);
	draw_mouse_pos(p);
}


/*void MultiView::set_selection_box(const base::optional<Box>& box) {
	selection_box = box;
	out_selection_changed();
	action_controller->update();
}*/


base::optional<Hover> MultiView::get_hover(MultiViewWindow* win, const vec2& m) const {
	vec3 tp;
	auto con = action_controller->get_hover(win, m, tp);
	if (con != ActionController::Constraint::UNDEFINED)
		return Hover{MultiViewType::ACTION_MANAGER, (int)con, tp};

	if (f_hover)
		return f_hover(win, m);
	return base::None;
}


vec3 MultiView::snap_v2(const vec3 &v, float d) {
	vec3 w;
	w.x = d * roundf(v.x / d);
	w.y = d * roundf(v.y / d);
	w.z = d * roundf(v.z / d);
	return w;
}

vec3 MultiView::snap_v(const vec3 &v) const {
	return snap_v2(v, active_window->get_grid_d());
}

float MultiView::snap_f(float f) const {
	float d = active_window->get_grid_d();
	return d * roundf(f / d);
}

vec3 MultiView::maybe_snap_v2(const vec3 &v, float d) const {
	if (snap_to_grid)
		return snap_v2(v, d);
	return v;
}

vec3 MultiView::maybe_snap_v(const vec3 &v) const {
	if (snap_to_grid)
		return snap_v(v);
	return v;
}

float MultiView::maybe_snap_f(float f) const {
	if (snap_to_grid)
		return snap_f(f);
	return f;
}

string MultiView::get_unit_by_zoom(vec3 &v) const {
	const char *units[] = {"y", "z", "a", "f", "p", "n", "\u00b5", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y"};
	float l = active_window->get_grid_d() * 10.1f;

	int n = floor(log10(l) / 3.0f);
	v /= pow(10.0f, n * 3);
	if ((n >= -8) and  (n <= 8))
		return units[n + 8];
	return format("*10^%d", n*3);
}

string MultiView::format_length(float l) const {
	vec3 v = vec3(l, 0, 0);
	string unit = get_unit_by_zoom(v);
	return f2s(v.x,2) + " " + unit;
}

void MultiView::draw_mouse_pos(Painter* p) {
	if (!hover)
		return;
	vec3 m = hover->tp;
	string unit = get_unit_by_zoom(m);
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;

	session->drawing_helper->draw_boxed_str(p, area.p11() - vec2(30,80), sx + "\n" + sy +  + "\n" + sz, 1);

#if 0 //HAS_LIB_GL
	if (mouse_win->type == VIEW_2D) {
		drawing_helper->draw_str(nix::target_width, nix::target_height - 60, sx + "\n" + sy, TextAlign::RIGHT);
	} else {
		drawing_helper->draw_str(nix::target_width, nix::target_height - 80, sx + "\n" + sy +  + "\n" + sz, TextAlign::RIGHT);
	}
#endif
}

vec3 MultiView::cursor_pos_3d(const vec2& m) const {
	return window.unproject(vec3(m, 0), view_port.pos);
}

void MultiView::set_allow_action(bool allow) {
	_allow_action = allow;
}

void MultiView::set_allow_select(bool allow) {
	_allow_select = allow;
}



MultiView::ViewPort::ViewPort(MultiView* _multi_view) {
	multi_view = _multi_view;
	pos = v_0;
	ang = quaternion::ID;
	radius = 100;
	cam = new Camera();
	cam->owner = new Entity;
	//cam->owner->ang = quaternion::rotation({1, 0, 0}, 0.33f);
	//cam->owner->pos = {1000,1000,-800};
	scene_view = new SceneView;
	scene_view->cam = cam;
}

void MultiView::ViewPort::move(const vec3& drel) {
	pos = pos + ang * drel * radius;
	out_changed();
}

void MultiView::ViewPort::rotate(const quaternion& qrel) {
	ang = ang * qrel;
	out_changed();
}

void MultiView::ViewPort::zoom(float factor) {
	radius /= factor;
	out_changed();
}

void MultiView::ViewPort::suggest_for_box(const Box& box) {
	pos = box.center();
	if (box.size() == v_0)
		radius = 200;
	else
		radius = box.size().length() * 0.7f;
	ang = quaternion::rotation({0.35f, 0, 0});
	out_changed();
}


