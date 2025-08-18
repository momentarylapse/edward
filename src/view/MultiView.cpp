//
// Created by Michael Ankele on 2025-01-20.
//

#include "MultiView.h"
#include "MultiViewWindow.h"
#include "MultiViewMeshEmitter.h"
#include "ActionController.h"
#include "DrawingHelper.h"
#include "SingleData.h"
#include <Session.h>
#include <action/ActionMultiView.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/math/mat3.h>
#include <lib/yrenderer/scene/SceneRenderer.h>
#include <lib/yrenderer/scene/MeshEmitter.h>
#include <lib/yrenderer/scene/pass/ShadowRenderer.h>
#include <lib/yrenderer/helper/CubeMapSource.h>
#include <lib/yrenderer/target/TextureRenderer.h>

#include "EdwardWindow.h"
#include <lib/ygraphics/graphics-impl.h>

#include "Mode.h"
#include "lib/yrenderer/scene/pass/CubeMapRenderer.h"





MultiView::MultiView(Session* s) :
		in_data_changed(this, [this] {
			if (!action_controller->performing_action()) {
				update_selection_box();
				hover = base::None;
			}
		}),
		view_port(this)
{
	session = s;
	ctx = session->ctx;
	window = new MultiViewWindow(this);
	active_window = window.get();
	hover_window = window.get();

	renderer = new MultiViewRenderer(session->ctx, this);

	action_controller = new ActionController(this);
	view_port.out_changed >> action_controller->in_view_changed;
	out_selection_changed >> action_controller->in_selection_changed;


	light_mode = LightMode::FollowCamera;
	default_light = new yrenderer::Light;
	default_light->init(White, -1, -1);
	default_light->_ang = quaternion::ID;
	default_light->enabled = true;
	default_light->allow_shadow = true;
	default_light->light.harshness = 0.5f;
	lights.add(default_light);

	shadow_renderer = new yrenderer::ShadowRenderer(session->ctx, view_port.scene_view.get(), 2048);
	shadow_renderer->add_emitter(new MultiViewShadowGeometryEmitter(this));
	view_port.scene_view->shadow_maps.add(shadow_renderer->cascades[0].depth_buffer);
	view_port.scene_view->shadow_maps.add(shadow_renderer->cascades[1].depth_buffer);
	//add_child(shadow_renderer.get());

	//cam_main = view_port.cam;
	cube_map_source = new yrenderer::CubeMapSource;
	cube_map_source->resolution = 256;
	cube_map_source->cube_map = new ygfx::CubeMap(cube_map_source->resolution, "rgba:i8");
	cube_map_renderer = new yrenderer::CubeMapRenderer(session->ctx, *view_port.scene_view.get());
	cube_map_renderer->add_emitter(new MultiViewBackgroundEmitter(this));
	cube_map_renderer->set_source(cube_map_source.get());
	view_port.scene_view->cube_map = cube_map_source->cube_map;

	for (int i=0; i<(int)MultiViewType::_NUM; i++)
		selection.set((MultiViewType)i, {});
}

MultiView::~MultiView() = default;

void MultiView::set_area(const rect& _area) {
	area = _area;
	area_native = {_area.p00() * session->win->ui_scale, _area.p11() * session->win->ui_scale};
	window->area = area;
	window->area_native = area_native;
}


MultiViewRenderer::MultiViewRenderer(yrenderer::Context *ctx, MultiView *mv) : Renderer(ctx, "multiview") {
	multi_view = mv;
}

void MultiViewRenderer::prepare(const yrenderer::RenderParams& params) {
	auto& view_port = multi_view->view_port;
	view_port.cam.ang = view_port.ang;
	view_port.cam.pos = view_port.pos - view_port.cam.ang * vec3::EZ * view_port.radius;
	view_port.cam.min_depth = view_port.radius * 0.01f;
	view_port.cam.max_depth = view_port.radius * 300;
	//view_port.cam.update_matrix_cache(area.width() / area.height());
	view_port.scene_view->shadow_box_size = view_port.radius * 8;

	multi_view->window->local_ang = view_port.ang;

	{
		if (multi_view->light_mode == MultiView::LightMode::FollowCamera)
			multi_view->default_light->_ang = view_port.ang;
		multi_view->lights = {multi_view->default_light};
	}

	multi_view->session->cur_mode->on_prepare_scene(params);

	view_port.scene_view->choose_lights(multi_view->lights);
	view_port.scene_view->choose_shadows();

	multi_view->window->prepare(params);

	if (multi_view->shadow_renderer)
		multi_view->shadow_renderer->render(params);

	if (multi_view->cube_map_renderer)
		multi_view->cube_map_renderer->render(params);

	//Renderer::prepare(params);
}

void MultiViewRenderer::draw(const yrenderer::RenderParams& params) {
	//engine.physical_aspect_ratio = pp->native_area.width() / pp->native_area.height();

	multi_view->window->draw(params);
}


void MultiView::on_mouse_move(const vec2& m, const vec2& d) {
	if (action_controller->on_mouse_move(m, d))
		return;

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
	selection.clear();
	for (int i=0; i<(int)MultiViewType::_NUM; i++)
		selection.set((MultiViewType)i, {});
	selection_box = base::None;
	out_selection_changed();
}

void MultiView::select_all() {
	for (auto& d: data_sets)
		for (int i=0; i<d.array->num; i++)
			selection[d.type].add(i);
	update_selection_box();
}

void MultiView::invert_selection() {
	for (auto& d: data_sets)
		for (int i=0; i<d.array->num; i++) {
			if (selection[d.type].contains(i))
				selection[d.type].erase(i);
			else
				selection[d.type].add(i);
		}
	update_selection_box();
}

void MultiView::select_in_rect(MultiViewWindow* win, const rect& _r) {
	const auto r = _r.canonical();
	if (f_select)
		selection = f_select(win, r);

	for (int i=0; i<(int)MultiViewType::_NUM; i++)
		if (!selection.contains((MultiViewType)i))
			selection.set((MultiViewType)i, {});

	if (session->win->is_key_pressed(xhui::KEY_CONTROL)) {
		// OR
		for (int i=0; i<(int)MultiViewType::_NUM; i++)
			for (int k: temp_selection[(MultiViewType)i])
				selection[(MultiViewType)i].add(k);
	} else if (session->win->is_key_pressed(xhui::KEY_SHIFT)) {
		// XOR
		for (int i=0; i<(int)MultiViewType::_NUM; i++)
			for (int k: temp_selection[(MultiViewType)i]) {
				if (selection[(MultiViewType)i].contains(k))
					selection[(MultiViewType)i].erase(k);
				else
					selection[(MultiViewType)i].add(k);
			}
	}

	update_selection_box();
}

base::set<int> MultiView::select_points_in_rect(MultiViewWindow* win, const rect& r, DynamicArray& array) {
	base::set<int> sel;
	for (int i=0; i<array.num; i++) {
		auto p = reinterpret_cast<multiview::SingleData*>(array.simple_element(i));
		if (r.inside(win->project(p->pos).xy()))
			sel.add(i);
	}
	return sel;
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
	if (f_make_selection_consistent)
		f_make_selection_consistent(selection);
	if (f_get_selection_box)
		selection_box = f_get_selection_box(selection);
	out_selection_changed();
}

base::optional<Box> MultiView::points_get_selection_box(const DynamicArray& _array, const base::set<int>& sel) {
	base::optional<Box> box;
	auto& array = const_cast<DynamicArray&>(_array);
	for (int i=0; i<array.num; i++) {
		auto p = reinterpret_cast<multiview::SingleData*>(array.simple_element(i));
		if (sel.contains(i)) {
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
	temp_selection = selection;

	if (!_allow_select)
		return;

	if (hover and hover->type == MultiViewType::ACTION_MANAGER) {
		auto data = session->cur_mode->get_data();
		//if (action_controller->on_left_button_down(m, data))
		//	return;
		if (f_create_action)
			action_controller->start_action(data, f_create_action(), hover->tp, (ActionController::Constraint)hover->index);

	} else if (hover) {
		if (session->win->is_key_pressed(xhui::KEY_SHIFT)) {
			// toggle
			if (selection[hover->type].contains(hover->index))
				selection[hover->type].erase(hover->index);
			else
				selection[hover->type].add(hover->index);
			update_selection_box();
		} else if (session->win->is_key_pressed(xhui::KEY_CONTROL)) {
			// add
			selection[hover->type].add(hover->index);
			update_selection_box();
		} else {
			// select exclusively
			clear_selection();
			selection[hover->type].add(hover->index);
			update_selection_box();
		}
	} else {
		if (!session->win->is_key_pressed(xhui::KEY_SHIFT) and !session->win->is_key_pressed(xhui::KEY_CONTROL))
			clear_selection();
	}
}

void MultiView::on_left_button_up(const vec2& m) {
	//action_controller->on_left_button_up(m);
	if (action_controller->performing_action()) {
		action_controller->end_action(true);
		update_selection_box();
		return;
	}
	selection_area = base::None;
	hover = get_hover(hover_window, m);
	possibly_selecting = false;
}



void MultiView::on_mouse_wheel(const vec2& m, const vec2& d) {
	view_port.zoom(exp(d.y * 0.1f), cursor_pos_3d(m));
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
	window->draw_post(p);
	action_controller->draw_post(p);
	draw_mouse_pos(p);
}


base::optional<Hover> MultiView::get_hover(MultiViewWindow* win, const vec2& m) const {
	vec3 tp;
	auto con = action_controller->get_hover(win, m, tp);
	if (con != ActionController::Constraint::UNDEFINED)
		return Hover{MultiViewType::ACTION_MANAGER, (int)con, tp};

	if (f_hover)
		if (auto h = f_hover(win, m))
			return h;

	if (window->area.inside(m)) {
		// grid? (included here, so we can see the cursor)
		if (auto p = grid_hover_point(m))
			return Hover{MultiViewType::GRID, -1, *p};

		// TODO camera focus plane?
	}

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

	drawing2d::draw_boxed_str(p, area.p11() - vec2(30,80), sx + "\n" + sy +  + "\n" + sz, 1);

#if 0 //HAS_LIB_GL
	if (mouse_win->type == VIEW_2D) {
		drawing_helper->draw_str(nix::target_width, nix::target_height - 60, sx + "\n" + sy, TextAlign::RIGHT);
	} else {
		drawing_helper->draw_str(nix::target_width, nix::target_height - 80, sx + "\n" + sy +  + "\n" + sz, TextAlign::RIGHT);
	}
#endif
}

vec3 MultiView::cursor_pos_3d(const vec2& m) const {
	// data? / grid?
	if (hover)
		return hover->tp;
	// grid?
//	if (auto p = grid_hover_point(m))
//		return *p;
	// camera focus plane
	return hover_window->unproject(m, view_port.pos);
}

base::optional<vec3> MultiView::grid_hover_point(const vec2& m) const {
	return window->grid_hover_point(m);
}

void MultiView::set_allow_action(bool allow) {
	action_controller->set_allowed(allow);
}

void MultiView::set_allow_select(bool allow) {
	_allow_select = allow;
}

void MultiView::set_show_grid(bool allow) {
	_show_grid = allow;
}



MultiView::ViewPort::ViewPort(MultiView* _multi_view) {
	multi_view = _multi_view;
	pos = v_0;
	ang = quaternion::ID;
	radius = 100;
	cam.ang = quaternion::rotation({1, 0, 0}, 0.33f);
	cam.pos = {1000,1000,-800};
	cam.fov = pi/4;
	cam.min_depth = 0.01f;
	cam.max_depth = 1000.0f;
	scene_view = new yrenderer::SceneView;
	scene_view->main_camera_params = cam;
}

void MultiView::ViewPort::move(const vec3& drel) {
	pos = pos + ang * drel * radius;
	out_changed();
}

void MultiView::ViewPort::rotate(const quaternion& qrel) {
	ang = ang * qrel;
	out_changed();
}

void MultiView::ViewPort::zoom(float factor, const base::optional<vec3>& focus_point) {
	if (focus_point) {
		if (factor > 1) {
			// zoom in -> re-focus camera on plane around <focus_point>
			auto w = multi_view->hover_window;
			vec3 pos1 = w->unproject(w->project(pos).xy(), *focus_point);
			float radius1 = (pos1 - w->view_pos()).length();

			radius = radius1;
			pos = pos1;
		}

		// fixed point: <focus_point>
		radius /= factor;
		const vec3 d = *focus_point - pos;
		pos = *focus_point - d / factor;
	} else {
		radius /= factor;
	}
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


