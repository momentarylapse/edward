/*
 * MultiView.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "MultiView.h"
#include "Window.h"
#include "ColorScheme.h"
#include "DrawingHelper.h"
#include "ActionController.h"
#include "CameraController.h"
#include "SingleData.h"
#include "LinearToSrgbRenderer.h"
#include "../EdwardWindow.h"
#include "../Session.h"
#include "../lib/os/time.h"
#include "../lib/nix/nix.h"
#include "../lib/math/plane.h"
#include "../lib/hui/config.h"
#include <y/helper/ResourceManager.h>
#include <y/renderer/Renderer.h>

#include "../lib/os/msg.h"

os::Timer timer;

namespace MultiView{



#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))
//#define MVGetSingleData(d, index)	( dynamic_cast<MultiViewSingleData*> ((char*)(d).data + (d).DataSingleSize * index))



void MultiView::Selection::reset() {
	meta = HOVER_NONE;
	index = set = type = -1;
	data = nullptr;
}

vec3 Camera::get_pos(bool allow_radius) const {
	vec3 p = pos;
	if (allow_radius and !ignore_radius)
		p -= radius * (ang * vec3::EZ);
	return p;
}

// FIXME only update on signal
void check_undef_view_stages(MultiView *mv) {
	for (auto data: mv->data) {
		char *p = (char*)data.data->data;
		for (int i=0; i<data.data->num; i++) {
			if (((SingleData*)p)->view_stage < 0)
				((SingleData*)p)->view_stage = mv->view_stage;
			p += data.data->element_size;
		}
	}
}

class Renderer : public ::Renderer {
public:
	MultiView* mv;
	explicit Renderer(MultiView* _mv) : ::Renderer("multiview") {
		mv = _mv;
	}

	void prepare(const RenderParams& p) override {
		mv->screen_scale = hui::get_event()->row_target; // EVIL!
		timer.reset();

		auto ctx = mv->ctx;
		auto drawing_helper = mv->drawing_helper;
		auto& area = mv->area;

		check_undef_view_stages(mv);

	}
	void draw(const RenderParams& p) override {

		mv->screen_scale = hui::get_event()->row_target; // EVIL!
		timer.reset();

		auto ctx = mv->ctx;
		auto drawing_helper = mv->drawing_helper;
		auto& area = mv->area;

		check_undef_view_stages(mv);

		using Selection = MultiView::Selection;

		area = p.area;

		using Selection = MultiView::Selection;
		area = p.area;
#if HAS_LIB_GL


		nix::clear_z();
		nix::set_projection_ortho_pixel();
		nix::set_z(true,true);
		mv->drawing_helper->set_color(scheme.TEXT);
		mv->drawing_helper->set_font(scheme.FONT_NAME, scheme.FONT_SIZE);


		if (!mv->mode3d) {
			mv->visible_windows = {mv->all_windows[0]};
			mv->all_windows[0]->dest = mv->area;
			mv->all_windows[0]->draw();
		} else if (mv->whole_window) {
			for (auto w: mv->all_windows)
				w->dest = rect(0,0,0,0);
			mv->visible_windows = {mv->active_win};
			mv->active_win->dest = mv->area;
			mv->active_win->draw();
		} else {
			float xm = area.x1 + area.width() * mv->window_partition_x;
			float ym = area.y1 + area.height() * mv->window_partition_y;
			float d = scheme.WINDOW_DIVIDER_THICKNESS / 2;
			mv->visible_windows = {mv->all_windows[0], mv->all_windows[1], mv->all_windows[2], mv->all_windows[3]};


			// top left
			mv->all_windows[0]->dest = rect(area.x1, xm-d+1, area.y1, ym-d+1);
			mv->all_windows[0]->draw();

			// top right
			mv->all_windows[1]->dest = rect(xm+d-1, area.x2, area.y1, ym-d+1);
			mv->all_windows[1]->draw();

			// bottom left
			mv->all_windows[2]->dest = rect(area.x1, xm-d+1, ym+d-1, area.y2);
			mv->all_windows[2]->draw();

			// bottom right
			mv->all_windows[3]->dest = rect(xm+d-1, area.x2, ym+d-1, area.y2);
			mv->all_windows[3]->draw();

			nix::set_scissor(nix::target_rect);

			nix::set_shader(ctx->default_2d.get());
			nix::bind_texture(0, nullptr);

			color c2 = scheme.hoverify(scheme.WINDOW_DIVIDER);
			drawing_helper->set_color((mv->hover.meta == Selection::HOVER_WINDOW_DIVIDER_Y or mv->hover.meta == Selection::HOVER_WINDOW_DIVIDER_CENTER) ? c2 : scheme.WINDOW_DIVIDER);
			drawing_helper->draw_rect(area.x1, area.x2, ym-d, ym+d, 0);
			drawing_helper->set_color((mv->hover.meta == Selection::HOVER_WINDOW_DIVIDER_X or mv->hover.meta == Selection::HOVER_WINDOW_DIVIDER_CENTER) ? c2 : scheme.WINDOW_DIVIDER);
			drawing_helper->draw_rect(xm-d, xm+d, area.y1, area.y2, 0);
		}

		nix::set_projection_ortho_pixel();
		if (mv->sel_rect.active)
			mv->sel_rect.draw(drawing_helper, mv->m);

		mv->cam_con->draw();

		drawing_helper->set_color(scheme.TEXT);
		nix::set_shader(ctx->default_2d.get());

		//if (session->win->input.inside_smart)
		// FIXME hui/gtk4 events!
			mv->draw_mouse_pos();

		mv->action_con->draw_post();

		mv->draw_mouse_pos();
#endif
#if HAS_LIB_VULKAN
		auto cb = p.command_buffer;
		cb->clear(area, {scheme.BACKGROUND}, 1);
		static int count = 0;
		count = loop(count + 10, 0, 1000);
		cb->clear({(float)count + 100,(float)count + 400,100,400}, {Red}, 1);
#endif
	}
};

MultiView::MultiView(Session *_s, bool mode3d) {
	session = _s;
	ctx = session->ctx;
	drawing_helper = session->drawing_helper;
	view_stage = 0;
	grid_enabled = true;
	wire_mode = false;
	snap_to_grid = false;
	light_enabled = false;
	whole_window = true;
	this->mode3d = mode3d;
	mouse_win = nullptr;
	active_win = nullptr;
	allow_mouse_actions = true;
	area = rect::ID;


	moving_cross_x = false;
	moving_cross_y = false;
	window_partition_x = 0.5f;
	window_partition_y = 0.5f;

	SPEED_MOVE = hui::config.get_float("MultiView.MoveSpeed", 20);
	SPEED_ZOOM_KEY = hui::config.get_float("MultiView.ZoomSpeedKey", 1.15f);
	SPEED_ZOOM_WHEEL = hui::config.get_float("MultiView.ZoomSpeedWheel", 0.10f);
	HOVER_RADIUS = hui::config.get_float("MultiView.HoverRadius", 8.0f);

	MIN_MOUSE_MOVE_TO_INTERACT = 5;
	MOUSE_ROTATION_SPEED = 0.0033f;

	allow_infinite_scrolling = hui::config.get_bool("MultiView.InfiniteScrolling", false);
#if HAS_LIB_GL
	ubo_light = new UniformBuffer();
#else
	//ubo_light = new UniformBuffer(sizeof(Li));
#endif

	if (mode3d) {
		all_windows.add(new Window(this, VIEW_BACK));
		all_windows.add(new Window(this, VIEW_LEFT));
		all_windows.add(new Window(this, VIEW_TOP));
		all_windows.add(new Window(this, VIEW_PERSPECTIVE));

		// Menu
		menu = new hui::Menu(session->win);
		menu->add(_("View"), "view_menu_sign");
		//menu->enableItem("view_menu_sign", false);
		menu->add_separator();
		menu->add(_("Right (-x)"), "view_right");
		menu->add(_("Left (x)"), "view_left");
		menu->add(_("Front (z)"), "view_front");
		menu->add(_("Back (-z)"), "view_back");
		menu->add(_("Top (-y)"), "view_top");
		menu->add(_("Bottom (y)"), "view_bottom");
		menu->add(_("Isometric"), "view_isometric");
		menu->add(_("Perspective"), "view_perspective");
	} else {
		all_windows.add(new Window(this, VIEW_2D));
	}
	action_con = new ActionController(this);
	cam_con = new CameraController(this);
	m = v = {0,0};
	screen_scale = 1.0f;
	holding_cursor = false;
	holding_x = holding_y = 0;
	allow_mouse_actions = true;
	allow_select = true;
	edit_coordinate_mode = CoordinateMode::GLOBAL;

	renderer = new LinearToSrgbRenderer(session);
	renderer->add_child(new Renderer(this));

	out_camera_changed >> create_sink([this] { out_redraw(); });

	reset();
}

MultiView::~MultiView() {
	hui::config.set_bool("MultiView.InfiniteScrolling", allow_infinite_scrolling);
	hui::config.set_float("MultiView.ZoomSpeedWheel", SPEED_ZOOM_WHEEL);
	hui::config.set_float("MultiView.ZoomSpeedKey", SPEED_ZOOM_KEY);
	hui::config.set_float("MultiView.MoveSpeed", SPEED_MOVE);
	for (auto w: all_windows)
		delete w;
	delete cam_con;
	delete action_con;
}



void MultiView::reset() {
	sel_rect.end();
	mouse_win = all_windows.back();
	active_win = mouse_win;

	view_moving = false;

	hover.reset();
	action_con->reset();

	clear_data(nullptr);
	reset_mouse_action();
	reset_view();
}

void MultiView::reset_view() {
	cam.pos = v_0;
	cam.ang = quaternion::ID;
	if (mode3d) {
		cam.radius = 100;
		cam.ang = quaternion::rotation_v(vec3(0.4f, 0, 0));
	} else {
		cam.radius = 1;
	}
	whole_window = true;
	grid_enabled = true;
	light_enabled = true;
	snap_to_grid = false;
	cam.ignore_radius = false;
	wire_mode = false;
	window_partition_x = 0.5f;
	window_partition_y = 0.5f;

	view_stage = 0;

	hover.reset();
	out_camera_changed();
	out_viewstage_changed();
	out_settings_changed();
	out_redraw();
}

void MultiView::reset_mouse_action() {
	action_con->cur_action = nullptr;
	action_con->action.reset();
	out_settings_changed();
	out_redraw();
}

void MultiView::clear_data(Data *_data) {
	data.clear();
	action_con->data = _data;
	if (!_data)
		reset_mouse_action();
}

void MultiView::add_data(int type, const DynamicArray &a, int flags) {
	DataSet d;
	d.type = type;
	d.data = (DynamicArray*)&a;
	d.selectable = (flags & FLAG_SELECT)>0;
	d.drawable = (flags & FLAG_DRAW)>0;
	d.indexable = (flags & FLAG_INDEX)>0;
	d.movable = (flags & FLAG_MOVE)>0;
	data.add(d);
}

void MultiView::set_hover_func(int type, HoverDistanceFunction f) {
	for (auto &d: data)
		if (d.type == type)
			d.func_hover = f;
}

void MultiView::set_view_stage(int *view_stage, bool allow_handle) {}



void MultiView::cam_zoom(float factor, bool mouse_rel) {
	vec3 mup;
	if (mouse_rel)
		mup = mouse_win->unproject({m,0}, cam.pos);
	cam.radius /= factor;
	if (mouse_rel)
		cam.pos += (1 - 1/factor) * (mup - cam.pos);
	action_con->update();
	out_redraw();
}

// dir: screen pixels
void MultiView::cam_move_pixel(Window *win, const vec3 &dir) {
	vec3 d = win->reflection_matrix * dir;
	cam_move(-(win->local_ang * d) / win->zoom());
}

void MultiView::cam_move(const vec3 &dpos) {
	cam.pos += dpos;
	out_camera_changed();
}

// dir: screen pixels...yap
void MultiView::cam_rotate_pixel(const vec3 &dir, bool cam_center) {
	vec3 dang = vec3(dir.y, dir.x, 0) * MOUSE_ROTATION_SPEED;
	// could have used reflection_matrix... but...only VIEW_PERSPECTIVE...
	cam_rotate(quaternion::rotation_v(dang), cam_center);
}

void MultiView::cam_rotate(const quaternion &dq, bool cam_center) {
	if (cam_center)
		cam.pos -= cam.radius * (cam.ang * vec3::EZ);
	cam.ang = cam.ang * dq;
	if (cam_center)
		cam.pos += cam.radius * (cam.ang * vec3::EZ);
	action_con->update();
	out_camera_changed();
}

void MultiView::set_view_box(const vec3 &min, const vec3 &max) {
	cam.pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.8f;// * ((float)NixScreenWidth / (float)nix::target_width);
	if (r > 0)
		cam.radius = r;
	out_camera_changed();
}

void MultiView::toggle_whole_window() {
	whole_window = !whole_window;
	action_con->update();
	out_camera_changed();
}

void MultiView::toggle_grid() {
	grid_enabled = !grid_enabled;
	out_settings_changed();
	out_redraw();
}

void MultiView::toggle_light() {
	light_enabled = !light_enabled;
	out_settings_changed();
	out_redraw();
}

void MultiView::toggle_wire() {
	wire_mode = !wire_mode;
	out_settings_changed();
	out_redraw();
}

void MultiView::toggle_snap_to_grid() {
	snap_to_grid = !snap_to_grid;
	out_settings_changed();
}

void MultiView::on_command(const string & id) {

	if (id == "select_all")
		select_all();
	if (id == "select_none")
		select_none();
	if (id == "invert_selection")
		invert_selection();

	if (id == "view_right")
		active_win->type = VIEW_RIGHT;
	if (id == "view_left")
		active_win->type = VIEW_LEFT;
	if (id == "view_front")
		active_win->type = VIEW_FRONT;
	if (id == "view_back")
		active_win->type = VIEW_BACK;
	if (id == "view_top")
		active_win->type = VIEW_TOP;
	if (id == "view_bottom")
		active_win->type = VIEW_BOTTOM;
	if (id == "view_perspective")
		active_win->type = VIEW_PERSPECTIVE;
	if (id == "view_isometric")
		active_win->type = VIEW_ISOMETRIC;
	if (id == "whole_window")
		toggle_whole_window();
	if (id == "grid")
		toggle_grid();
	if (id == "snap_to_grid")
		toggle_snap_to_grid();
	if (id == "light")
		toggle_light();
	if (id == "wire")
		toggle_wire();

	if (id == "view_push")
		view_stage_push();
	if (id == "view_pop")
		view_stage_pop();
}

void MultiView::on_mouse_wheel(const vec2& scroll) {
	cam_con->on_mouse_wheel(scroll);
}

void MultiView::on_gesture_zoom_begin() {
	cam_con->on_gesture_zoom_begin();
}

void MultiView::on_gesture_zoom(float factor) {
	cam_con->on_gesture_zoom(factor);
}

void MultiView::on_mouse_enter(const vec2& m) {
	out_redraw();
}

void MultiView::on_mouse_leave() {
	//notify(MESSAGE_UPDATE);
	on_mouse_move(vec2::ZERO);
}

void activate_next_window(MultiView *mv) {
	if (mv->whole_window)
		return;
	for (int i=0; i<mv->all_windows.num; i++)
		if (mv->all_windows[i] == mv->active_win) {
			mv->active_win = mv->all_windows[(i+1)%mv->all_windows.num];
			mv->out_settings_changed();
			break;
		}

}


void MultiView::on_key_down(int k) {
	if ((k == hui::KEY_PLUS) or (k == hui::KEY_NUM_ADD))
		cam_zoom(SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if ((k == hui::KEY_MINUS) or (k == hui::KEY_NUM_SUBTRACT))
		cam_zoom(1.0f / SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if (k == hui::KEY_RIGHT)
		cam_move_pixel(active_win, -vec3::EX * SPEED_MOVE);
	if (k == hui::KEY_LEFT)
		cam_move_pixel(active_win,  vec3::EX * SPEED_MOVE);
	if (k == hui::KEY_UP)
		cam_move_pixel(active_win,  vec3::EY * SPEED_MOVE);
	if (k == hui::KEY_DOWN)
		cam_move_pixel(active_win, -vec3::EY * SPEED_MOVE);
	if (k == hui::KEY_SHIFT + hui::KEY_UP)
		cam_move_pixel(active_win, -vec3::EZ * SPEED_MOVE);
	if (k == hui::KEY_SHIFT + hui::KEY_DOWN)
		cam_move_pixel(active_win,  vec3::EZ * SPEED_MOVE);
	if (k == hui::KEY_ESCAPE)
		action_con->end_action(false);
	if (k == hui::KEY_TAB)
		activate_next_window(this);
}


MultiView::SelectionMode get_select_mode(Session *session) {
	if (session->win->get_key(hui::KEY_CONTROL))
		return MultiView::SelectionMode::ADD;
	if (session->win->get_key(hui::KEY_SHIFT))
		return MultiView::SelectionMode::INVERT;
	return MultiView::SelectionMode::SET;
}

void MultiView::on_left_button_down(const vec2& m) {
	update_mouse();

	get_hover();

	// menu for selection of view type
	if (hover.meta == Selection::HOVER_WINDOW_LABEL) {
		active_win = mouse_win;
	} else if (hover.meta == Selection::HOVER_WINDOW_DIVIDER_X) {
		moving_cross_x = true;
	} else if (hover.meta == Selection::HOVER_WINDOW_DIVIDER_Y) {
		moving_cross_y = true;
	} else if (hover.meta == Selection::HOVER_WINDOW_DIVIDER_CENTER) {
		moving_cross_x = true;
		moving_cross_y = true;
	} else if (hover.meta == Selection::HOVER_CAMERA_CONTROLLER) {
		cam_con->on_left_button_down();
	} else if (hover.meta == Selection::HOVER_ACTION_CONTROLLER) {
		active_win = mouse_win;
		action_con->on_left_button_down();
	} else {
		active_win = mouse_win;
		if (action_con->action.locked) {
			if (action_con->action.mode == ACTION_SELECT) {
				if (allow_select) {
					get_selected(get_select_mode(session));
					sel_rect.start_later(m);
				}
			} else if (allow_mouse_actions and hover_selected()) {
				action_con->start_action(active_win, hover.point, ActionController::Constraint::FREE);
			}
		} else {
			if (allow_select) {
				if (hover_selected() and (get_select_mode(session) == MultiView::SelectionMode::SET)) {
					action_con->start_action(active_win, hover.point, ActionController::Constraint::FREE);
				} else {
					get_selected(get_select_mode(session));
					sel_rect.start_later(m);
				}
			}
		}
	}
}



void MultiView::on_middle_button_down(const vec2& m) {
	active_win = mouse_win;

// move camera?
	if (allow_infinite_scrolling)
		hold_cursor(true);
	view_moving = true;

	out_redraw();
}



void MultiView::on_right_button_down(const vec2& m) {
	if (hover.meta == hover.HOVER_WINDOW_LABEL) {
		active_win = mouse_win;
		menu->open_popup(session->win);
	} else {
		active_win = mouse_win;

// move camera?
		if (allow_infinite_scrolling)
			hold_cursor(true);
		view_moving = true;
	}

	out_redraw();
}



void MultiView::on_middle_button_up(const vec2& m) {
	if (view_moving) {
		view_moving = false;
		hold_cursor(false);
	}
}



void MultiView::on_right_button_up(const vec2& m) {
	if (view_moving) {
		view_moving = false;
		hold_cursor(false);
	}
}



void MultiView::on_key_up(int key_code) {
}



void MultiView::on_left_button_up(const vec2& m) {
	end_selection_rect();
	moving_cross_x = false;
	moving_cross_y = false;

	action_con->on_left_button_up();
	cam_con->on_left_button_up();
}



void MultiView::update_mouse() {
	m = hui::get_event()->m * screen_scale;
	v = hui::get_event()->d * screen_scale;

	lbut = hui::get_event()->lbut;
	mbut = hui::get_event()->mbut;
	rbut = hui::get_event()->rbut;

	if (allow_mouse_actions)
		if (cam_con->is_mouse_over())
			return;

	// which window is the cursor in?
	for (auto w: all_windows)
		if (w->dest.inside(m))
			mouse_win = w;
	if (!mode3d) {
		mouse_win = all_windows[0];
	}
}


void MultiView::on_mouse_move(const vec2& m) {
	update_mouse();

	if (action_con->in_use()) {
		action_con->on_mouse_move();
	} else if (cam_con->in_use()) {
		cam_con->on_mouse_move();
	} else if (sel_rect.active and allow_select) {
		select_all_in_rectangle(get_select_mode(session));
	} else if (view_moving) {
		int t = active_win->type;
		if ((t == VIEW_PERSPECTIVE) or (t == VIEW_ISOMETRIC)) {
	// camera rotation
			cam_rotate_pixel({v,0}, mbut or session->win->get_key(hui::KEY_CONTROL));
		} else {
	// camera translation
			cam_move_pixel(active_win, {v,0});
		}
	} else if (moving_cross_x or moving_cross_y) {
		if (moving_cross_x)
			window_partition_x = clamp((m.x - area.x1) / area.width(), 0.01f, 0.99f);
		if (moving_cross_y)
			window_partition_y = clamp((m.y - area.y1) / area.height(), 0.01f, 0.99f);
	} else if (sel_rect.dist >= 0 and allow_select) {
		sel_rect.dist += abs(v.x) + abs(v.y);
		if (sel_rect.dist >= MIN_MOUSE_MOVE_TO_INTERACT)
			start_selection_rect();
	} else {

		get_hover();

	}



	// ignore mouse, while "holding"
	if (holding_cursor) {
		if (fabs(m.x - holding_x) + fabs(m.y - holding_y) > 100)
			session->win->set_cursor_pos(holding_x, holding_y);
	}

	out_redraw();
}




void MultiView::start_selection_rect() {
	sel_rect.active = true;
	sel_rect.dist = -1;

	// reset selection data
	for (DataSet &d: data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++) {
				SingleData* sd = MVGetSingleData(d,i);
				sd->m_old = sd->is_selected;
			}

	out_redraw();
}

void MultiView::end_selection_rect() {
	sel_rect.end();
	out_redraw();
}


void MultiView::force_redraw() {
	out_redraw();
}


string MultiView::get_unit_by_zoom(vec3 &v) {
	const char *units[] = {"y", "z", "a", "f", "p", "n", "\u00b5", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y"};
	float l = active_win->get_grid_d() * 10.1f;

	int n = floor(log10(l) / 3.0f);
	v /= pow(10.0f, n * 3);
	if ((n >= -8) and  (n <= 8))
		return units[n + 8];
	return format("*10^%d", n*3);
}

string MultiView::format_length(float l) {
	vec3 v = vec3(l, 0, 0);
	string unit = get_unit_by_zoom(v);
	return f2s(v.x,2) + " " + unit;
}

void MultiView::draw_mouse_pos() {
	vec3 m = get_cursor();
	string unit = get_unit_by_zoom(m);
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;

#if HAS_LIB_GL
	if (mouse_win->type == VIEW_2D) {
		drawing_helper->draw_str(nix::target_width, nix::target_height - 60, sx + "\n" + sy, TextAlign::RIGHT);
	} else {
		drawing_helper->draw_str(nix::target_width, nix::target_height - 80, sx + "\n" + sy +  + "\n" + sz, TextAlign::RIGHT);
	}
#endif
}


void MultiView::SelectionRect::start_later(const vec2 &m) {
	pos0 = m;
	dist = 0;
}

void MultiView::SelectionRect::end() {
	active = false;
	dist = -1;
}

void MultiView::SelectionRect::draw(DrawingHelper *drawing_helper, const vec2 &m) {
#if HAS_LIB_GL
	nix::set_z(false, false);
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::bind_texture(0, nullptr);
	drawing_helper->set_color(scheme.SELECTION_RECT);
	nix::set_cull(nix::CullMode::NONE);
	nix::set_shader(drawing_helper->ctx->default_2d.get());
	drawing_helper->draw_rect(m.x, pos0.x, m.y, pos0.y, 0);
	nix::set_cull(nix::CullMode::BACK);
	drawing_helper->set_color(scheme.SELECTION_RECT_BOUNDARY);
	drawing_helper->set_line_width(scheme.LINE_WIDTH_THIN);
	drawing_helper->draw_line_2d(pos0.x, pos0.y, pos0.x, m.y, 0);
	drawing_helper->draw_line_2d(m.x, pos0.y, m.x, m.y, 0);
	drawing_helper->draw_line_2d(pos0.x, pos0.y, m.x, pos0.y, 0);
	drawing_helper->draw_line_2d(pos0.x, m.y, m.x, m.y, 0);
	nix::disable_alpha();
	nix::set_z(true, true);
#endif
}

rect MultiView::SelectionRect::get(const vec2 &m) {
	return rect(min(m.x, pos0.x), max(m.x, pos0.x), min(m.y, pos0.y), max(m.y, pos0.y));
}

void MultiView::set_mouse_action(const string & name, int mode, bool locked) {
	if (!mode3d and (mode == ACTION_ROTATE))
		mode = ACTION_ROTATE_2D;
	action_con->action.name = name;
	action_con->action.mode = mode;
	action_con->action.locked = locked;
	action_con->show(need_action_controller());
	out_settings_changed();
}

bool MultiView::need_action_controller() {
	if (!has_selection())
		return false;
	if (!allow_mouse_actions)
		return false;
	if (action_con->action.mode == ACTION_SELECT)
		return false;
	return true;
}

void MultiView::select_all() {
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++) {
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = true;
		}
	action_con->show(need_action_controller());
	out_settings_changed();
}

void MultiView::select_none() {
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++) {
			SingleData* sd = MVGetSingleData(d, i);
			sd->is_selected = false;
		}
	action_con->show(need_action_controller());
	out_selection_changed();
}

void MultiView::invert_selection() {
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++) {
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = !sd->is_selected;
		}
	action_con->show(need_action_controller());
	out_selection_changed();
}

bool MultiView::has_selection() {
	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++) {
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->is_selected)
				return true;
		}
	return false;
}

vec3 MultiView::get_selection_center() {
	vec3 min = v_0, max = v_0;
	bool first = true;
	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++) {
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->is_selected) {
				if (first) {
					min = max = sd->pos;
					first = false;
				} else {
					min._min(sd->pos);
					max._max(sd->pos);
				}
			}
		}
	return (min + max) / 2;
}

vec3 MultiView::snap_v2(const vec3 &v, float d) {
	vec3 w;
	w.x = d * roundf(v.x / d);
	w.y = d * roundf(v.y / d);
	w.z = d * roundf(v.z / d);
	return w;
}

vec3 MultiView::snap_v(const vec3 &v) {
	return snap_v2(v, active_win->get_grid_d());
}

float MultiView::snap_f(float f) {
	float d = active_win->get_grid_d();
	return d * roundf(f / d);
}

vec3 MultiView::maybe_snap_v2(const vec3 &v, float d) {
	if (snap_to_grid)
		return snap_v2(v, d);
	return v;
}

vec3 MultiView::maybe_snap_v(const vec3 &v) {
	if (snap_to_grid)
		return snap_v(v);
	return v;
}

float MultiView::maybe_snap_f(float f) {
	if (snap_to_grid)
		return snap_f(f);
	return f;
}

void MultiView::set_edit_coordinate_mode(CoordinateMode mode) {
	edit_coordinate_mode = mode;
	out_settings_changed();
}

vec3 MultiView::get_cursor() {
	if (hover.data)
		return hover.point;
	if (edit_coordinate_mode == CoordinateMode::GLOBAL) {
		plane pl = plane(mouse_win->active_grid_direction(), 0);
		vec3 tp;
		if (pl.intersect_line(mouse_win->unproject({m,0}), mouse_win->unproject({m,0.999999f}), tp))
			return maybe_snap_v(tp);
	}
	return maybe_snap_v(mouse_win->unproject({m,0}, cam.pos));
}

vec3 MultiView::get_cursor(const vec3 &depth_reference) {
	return mouse_win->unproject({m,0}, depth_reference);
}


void MultiView::get_hover() {
	hover.reset();

	//if (!session->win->input.inside_smart)
	//	return;


	if (menu and (mouse_win->name_dest.inside(m))) {
		hover.meta = Selection::HOVER_WINDOW_LABEL;
		return;
	}
	if (cam_con->is_mouse_over()) {
		hover.meta = Selection::HOVER_CAMERA_CONTROLLER;
		return;
	}
	if (!whole_window) {
		float xm = all_windows[0]->dest.x2;
		float ym = all_windows[0]->dest.y2;
		if ((m.x >= xm - 5) and (m.x <= xm + 5) and (m.y >= ym - 5) and (m.y <= ym + 5)) {
			hover.meta = Selection::HOVER_WINDOW_DIVIDER_CENTER;
			return;
		} else if ((m.x >= xm - 5) and (m.x <= xm + 5)) {
			hover.meta = Selection::HOVER_WINDOW_DIVIDER_X;
			return;
		} else if ((m.y >= ym - 5) and (m.y <= ym + 5)) {
			hover.meta = Selection::HOVER_WINDOW_DIVIDER_Y;
			return;
		}
	}
	if (allow_mouse_actions and (action_con->get_hover(hover.point) != ActionController::Constraint::UNDEFINED)) {
		hover.meta = Selection::HOVER_ACTION_CONTROLLER;
		return;
	}
	float z_min = 1;
	float dist_min = HOVER_RADIUS;
	foreachi(DataSet &d, data, di)
		if (d.selectable)
			for (int i=0; i<d.data->num; i++) {
				SingleData* sd = MVGetSingleData(d, i);
				if (sd->view_stage < view_stage)
					continue;

				// trace
				float z;
				vec3 tp, mop;
				float hover_dist;
				if (d.func_hover)
					hover_dist = d.func_hover(sd, mouse_win, m, tp, z);
				else
					hover_dist = sd->hover_distance(mouse_win, m, tp, z);

				if (hover_dist < 0)
					continue;
				if (hover_dist > dist_min)
					continue;
				if (z < z_min) {
					z_min = z;
					mop = tp;
				} else {
					if (sd->is_selected) {
						mop = tp;
					} else {
						continue;
					}
				}
				if (hover_dist > 0)
					dist_min = hover_dist;
				hover.index = i;
				hover.set = di;
				hover.type = d.type;
				hover.point = mop;
				hover.meta = Selection::HOVER_DATA;
				hover.data = sd;
				if (sd->is_selected)
					return;
			}
}

bool MultiView::hover_selected() {
	if (hover.index < 0)
		return false;
	return hover.data->is_selected;
}

bool MultiView::has_selectable_data() {
	for (DataSet &d: data)
		if (d.selectable)
			return true;
	return false;
}

void MultiView::get_selected(SelectionMode mode) {
	if ((hover.index < 0) or (hover.type < 0)) {
		if (mode == SelectionMode::SET)
			select_none();
	} else {
		SingleData* sd = MVGetSingleData(data[hover.set], hover.index);
		if (sd->is_selected) {
			if (mode == SelectionMode::INVERT) {
				sd->is_selected=false;
			}
		} else {
			if (mode == SelectionMode::SET) {
				select_none();
				sd->is_selected=true;
			} else {
				sd->is_selected=true;
			}
		}
	}
	action_con->show(need_action_controller());
	out_selection_changed();
}

void MultiView::selection_changed_manually() {
	action_con->show(need_action_controller());
	out_selection_changed();
}

void MultiView::select_all_in_rectangle(SelectionMode mode) {
	// reset data
	select_none();

	rect r = sel_rect.get(m);

	// select
	for (DataSet &d: data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++) {
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage<view_stage)
					continue;

				// selected?
				sd->m_delta = sd->in_rect(active_win, r);

				// add the selection layers
				if (mode == SelectionMode::INVERT)
					sd->is_selected = (sd->m_old and !sd->m_delta) or (!sd->m_old and sd->m_delta);
				else if (mode == SelectionMode::ADD)
					sd->is_selected = (sd->m_old or sd->m_delta);
				else
					sd->is_selected = sd->m_delta;
			}

	action_con->show(need_action_controller());
	out_selection_changed();
}

void MultiView::hold_cursor(bool holding) {
	holding_x = m.x;
	holding_y = m.y;
	holding_cursor = holding;
	session->win->show_cursor(!holding);
}



void MultiView::add_message_3d(const string &str, const vec3 &pos) {
	Message3d m;
	m.str = str;
	m.pos = pos;
	message3d.add(m);
}

void MultiView::set_allow_action(bool allow) {
	allow_mouse_actions = allow;
	action_con->show(need_action_controller());
	out_settings_changed();
}

void MultiView::set_allow_select(bool allow) {
	allow_select = allow;
	out_settings_changed();
}

void MultiView::push_settings() {
	Settings s;
	s.allow_select = allow_select;
	s.allow_action = allow_mouse_actions;
	s.action_name = action_con->action.name;
	s.action_mode = action_con->action.mode;
	s.action_locked = action_con->action.locked;
	settings_stack.add(s);
}

void MultiView::pop_settings() {
	Settings s = settings_stack.pop();
	allow_select = s.allow_select;
	allow_mouse_actions = s.allow_action;
	action_con->action.name = s.action_name;
	action_con->action.mode = s.action_mode;
	action_con->action.locked = s.action_locked;
	action_con->show(need_action_controller());
	out_settings_changed();
}

void MultiView::view_stage_push() {
	view_stage ++;

	for (DataSet &d: data)
		//if (d.selectable)
			for (int i=0;i<d.data->num;i++) {
				SingleData* sd = MVGetSingleData(d,i);
				if (sd->is_selected)
					sd->view_stage = view_stage;
			}
	out_viewstage_changed();
}

void MultiView::view_stage_pop() {
	if (view_stage <= 0)
		return;
	view_stage --;
	for (DataSet &d: data)
		//if (d.selectable)
			for (int i=0;i<d.data->num;i++) {
				SingleData* sd = MVGetSingleData(d,i);
				if (sd->view_stage > view_stage)
					sd->view_stage = view_stage;
			}
	out_viewstage_changed();
}

void MultiView::reset_message_3d() {
	message3d.clear();
}

void MultiView::set_light(Window *win, const vec3 &dir, const color &col, float harshness) {
#if HAS_LIB_GL
	nix::BasicLight l;
	l.proj = mat4::ID;
	l.dir = win->local_ang.bar() * dir;
	l.col = col;
	l.radius = -1;
	l.theta = pi;
	l.harshness = harshness;
	ubo_light->update(&l, sizeof(l));
	nix::bind_uniform_buffer(1, ubo_light);
#endif
}

}
;

