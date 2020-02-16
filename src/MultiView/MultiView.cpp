/*
 * MultiView.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../Edward.h"
#include "MultiView.h"
#include "Window.h"
#include "ColorScheme.h"
#include "DrawingHelper.h"
#include "ActionController.h"
#include "CameraController.h"
#include "SingleData.h"
#include "../lib/nix/nix.h"


hui::Timer timer;

namespace MultiView{

nix::Shader *shader_selection = NULL;
nix::Shader *shader_lines_3d = NULL;
nix::Shader *shader_lines_3d_colored = NULL;
nix::Shader *shader_lines_3d_colored_wide = NULL;



#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))
//#define MVGetSingleData(d, index)	( dynamic_cast<MultiViewSingleData*> ((char*)(d).data + (d).DataSingleSize * index))



const string MultiView::MESSAGE_UPDATE = "Update";
const string MultiView::MESSAGE_SELECTION_CHANGE = "SelectionChange";
const string MultiView::MESSAGE_SETTINGS_CHANGE = "SettingsChange";
const string MultiView::MESSAGE_CAMERA_CHANGE = "CameraChange";
const string MultiView::MESSAGE_VIEWSTAGE_CHANGE = "ViewStageChange";
const string MultiView::MESSAGE_ACTION_START = "ActionStart";
const string MultiView::MESSAGE_ACTION_UPDATE = "ActionUpdate";
const string MultiView::MESSAGE_ACTION_ABORT = "ActionAbort";
const string MultiView::MESSAGE_ACTION_EXECUTE = "ActionExecute";


void MultiView::Selection::reset()
{
	meta = HOVER_NONE;
	index = set = type = -1;
	data = NULL;
}

MultiView::MultiView(bool mode3d) :
	Observable("MultiView")
{

	view_stage = 0;
	grid_enabled = true;
	wire_mode = false;
	snap_to_grid = false;
	light = -1;
	light_enabled = false;
	whole_window = false;
	this->mode3d = mode3d;
	mouse_win = NULL;
	active_win = NULL;
	allow_mouse_actions = true;
	area = rect::ID;


	moving_cross_x = false;
	moving_cross_y = false;
	window_partition_x = 0.5f;
	window_partition_y = 0.5f;

	SPEED_MOVE = 20;
	SPEED_ZOOM_KEY = 1.15f;
	SPEED_ZOOM_WHEEL = 1.15f;

	MIN_MOUSE_MOVE_TO_INTERACT = 5;
	MOUSE_ROTATION_SPEED = 0.0033f;

	allow_infinite_scrolling = hui::Config.get_bool("MultiView.InfiniteScrolling", false);

	if (mode3d){
		win.add(new Window(this, VIEW_BACK));
		win.add(new Window(this, VIEW_LEFT));
		win.add(new Window(this, VIEW_TOP));
		win.add(new Window(this, VIEW_PERSPECTIVE));
		light = 0;

		// Menu
		menu = new hui::Menu;
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
	}else{
		win.add(new Window(this, VIEW_2D));
		light = -1;
	}
	action_con = new ActionController(this);
	cam_con = new CameraController(this);
	m = v = v_0;
	holding_cursor = false;
	holding_x = holding_y = 0;
	allow_mouse_actions = true;
	allow_select = true;


	if (!shader_lines_3d)
		shader_lines_3d = nix::Shader::load(app->directory_static + "shader/lines-3d.shader");
	if (!shader_lines_3d_colored)
		shader_lines_3d_colored = nix::Shader::load(app->directory_static + "shader/lines-3d-colored.shader");
	if (!shader_lines_3d_colored_wide)
		shader_lines_3d_colored_wide = nix::Shader::load(app->directory_static + "shader/lines-3d-colored-wide.shader");
	if (!shader_selection)
		shader_selection = nix::Shader::load(app->directory_static + "shader/selection.shader");

	reset();
}

MultiView::~MultiView()
{
	hui::Config.get_bool("MultiView.InfiniteScrolling", allow_infinite_scrolling);
	for (auto w: win)
		delete w;
	delete cam_con;
	delete action_con;
}



void MultiView::reset()
{
	sel_rect.end();
	mouse_win = win.back();
	active_win = mouse_win;

	view_moving = false;

	hover.reset();
	action_con->reset();

	clear_data(NULL);
	reset_mouse_action();
	reset_view();
}

void MultiView::reset_view()
{
	cam.pos = v_0;
	cam.ang = quaternion::ID;
	if (mode3d)
		cam.radius = 100;
	else
		cam.radius = 1;
	whole_window = false;
	grid_enabled = true;
	light_enabled = true;
	snap_to_grid = false;
	cam.ignore_radius = false;
	wire_mode = false;
	window_partition_x = 0.5f;
	window_partition_y = 0.5f;

	view_stage = 0;

	hover.reset();
	notify(MESSAGE_CAMERA_CHANGE);
	notify(MESSAGE_VIEWSTAGE_CHANGE);
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::reset_mouse_action()
{
	action_con->cur_action = NULL;
	action_con->action.reset();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::clear_data(Data *_data)
{
	data.clear();
	action_con->data = _data;
	if (!_data)
		reset_mouse_action();
}

void MultiView::add_data(int type, const DynamicArray & a, void *user_data, int flags)
{
	DataSet d;
	d.type = type;
	d.data = (DynamicArray*)&a;
	d.user_data = user_data;
	d.selectable = (flags & FLAG_SELECT)>0;
	d.drawable = (flags & FLAG_DRAW)>0;
	d.indexable = (flags & FLAG_INDEX)>0;
	d.movable = (flags & FLAG_MOVE)>0;
	data.add(d);
}

void MultiView::set_hover_func(int type, HoverFunction *f)
{
	for (auto &d: data)
		if (d.type == type)
			d.func_hover = f;
}

void MultiView::set_view_stage(int *view_stage, bool allow_handle)
{}



void MultiView::cam_zoom(float factor, bool mouse_rel)
{
	vector mup;
	if (mouse_rel)
		mup = mouse_win->unproject(m, cam.pos);
	cam.radius /= factor;
	if (mouse_rel)
		cam.pos += (1 - 1/factor) * (mup - cam.pos);
	action_con->update();
	notify(MESSAGE_UPDATE);
}

// dir: screen pixels
void MultiView::cam_move_pixel(const vector &dir) {
	vector d = active_win->reflection_matrix * dir;
	cam_move(-(active_win->local_ang * d) / active_win->zoom());
}

void MultiView::cam_move(const vector &dpos) {
	cam.pos += dpos;
	notify(MESSAGE_CAMERA_CHANGE);
}

// dir: screen pixels...yap
void MultiView::cam_rotate_pixel(const vector &dir, bool cam_center) {
	vector dang = vector(dir.y, dir.x, 0) * MOUSE_ROTATION_SPEED;
	// could have used reflection_matrix... but...only VIEW_PERSPECTIVE...
	cam_rotate(quaternion::rotation_v(dang), cam_center);
}

void MultiView::cam_rotate(const quaternion &dq, bool cam_center) {
	if (cam_center)
		cam.pos -= cam.radius * (cam.ang * vector::EZ);
	cam.ang = cam.ang * dq;
	if (cam_center)
		cam.pos += cam.radius * (cam.ang * vector::EZ);
	action_con->update();
	notify(MESSAGE_CAMERA_CHANGE);
}

void MultiView::set_view_box(const vector &min, const vector &max)
{
	cam.pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.3f;// * ((float)NixScreenWidth / (float)nix::target_width);
	if (r > 0)
		cam.radius = r;
	notify(MESSAGE_CAMERA_CHANGE);
}

void MultiView::toggle_whole_window()
{
	whole_window = !whole_window;
	action_con->update();
	notify(MESSAGE_CAMERA_CHANGE);
}

void MultiView::toggle_grid()
{
	grid_enabled = !grid_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::toggle_light()
{
	light_enabled = !light_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::toggle_wire()
{
	wire_mode = !wire_mode;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::toggle_snap_to_grid()
{
	snap_to_grid = !snap_to_grid;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::on_command(const string & id)
{
	notify_begin();

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
	notify_end();
}

void MultiView::on_mouse_wheel()
{
	notify_begin();
	hui::Event *e = hui::GetEvent();

	// mouse wheel -> zoom
	if (e->scroll_y > 0)
		cam_zoom(SPEED_ZOOM_WHEEL, mouse_win->type != VIEW_PERSPECTIVE);
	if (e->scroll_y < 0)
		cam_zoom(1.0f / SPEED_ZOOM_WHEEL, mouse_win->type != VIEW_PERSPECTIVE);
	notify_end();
}

void MultiView::on_mouse_enter()
{
	notify(MESSAGE_UPDATE);
}

void MultiView::on_mouse_leave()
{
	//notify(MESSAGE_UPDATE);
	on_mouse_move();
}

void activate_next_window(MultiView *mv)
{
	if (mv->whole_window)
		return;
	for (int i=0; i<mv->win.num; i++)
		if (mv->win[i] == mv->active_win){
			mv->active_win = mv->win[(i+1)%mv->win.num];
			mv->notify(mv->MESSAGE_SETTINGS_CHANGE);
			break;
		}

}


void MultiView::on_key_down(int k)
{
	notify_begin();

	if ((k == hui::KEY_ADD) or (k == hui::KEY_NUM_ADD))
		cam_zoom(SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if ((k == hui::KEY_SUBTRACT) or (k == hui::KEY_NUM_SUBTRACT))
		cam_zoom(1.0f / SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if (k == hui::KEY_RIGHT)
		cam_move_pixel(-vector::EX * SPEED_MOVE);
	if (k == hui::KEY_LEFT)
		cam_move_pixel( vector::EX * SPEED_MOVE);
	if (k == hui::KEY_UP)
		cam_move_pixel( vector::EY * SPEED_MOVE);
	if (k == hui::KEY_DOWN)
		cam_move_pixel(-vector::EY * SPEED_MOVE);
	if (k == hui::KEY_SHIFT + hui::KEY_UP)
		cam_move_pixel( vector::EZ * SPEED_MOVE);
	if (k == hui::KEY_SHIFT + hui::KEY_DOWN)
		cam_move_pixel(-vector::EZ * SPEED_MOVE);
	if (k == hui::KEY_ESCAPE)
		action_con->end_action(false);
	if (k == hui::KEY_TAB)
		activate_next_window(this);
	notify_end();
}


int get_select_mode()
{
	if (ed->get_key(hui::KEY_CONTROL))
		return MultiView::SELECT_ADD;
	if (ed->get_key(hui::KEY_SHIFT))
		return MultiView::SELECT_INVERT;
	return MultiView::SELECT_SET;
}

void MultiView::on_left_button_down()
{
	notify_begin();
	update_mouse();

	get_hover();

	// menu for selection of view type
	if (hover.meta == hover.HOVER_WINDOW_LABEL){
		active_win = mouse_win;
	}else if (hover.meta == hover.HOVER_WINDOW_DIVIDER_X){
		moving_cross_x = true;
	}else if (hover.meta == hover.HOVER_WINDOW_DIVIDER_Y){
		moving_cross_y = true;
	}else if (hover.meta == hover.HOVER_WINDOW_DIVIDER_CENTER){
		moving_cross_x = true;
		moving_cross_y = true;
	}else if (hover.meta == hover.HOVER_CAMERA_CONTROLLER){
		cam_con->onLeftButtonDown();
	}else if (hover.meta == hover.HOVER_ACTION_CONTROLLER){
		active_win = mouse_win;
		action_con->on_left_button_down();
	}else{
		active_win = mouse_win;
		if (action_con->action.locked){
			if (action_con->action.mode == ACTION_SELECT){
				if (allow_select){
					get_selected(get_select_mode());
					sel_rect.start_later(m);
				}
			}else if (allow_mouse_actions and hover_selected()){
				action_con->start_action(active_win, hover.point, ACTION_CONSTRAINTS_NONE);
			}
		}else{
			if (allow_select){
				if (hover_selected() and (get_select_mode() == MultiView::SELECT_SET)){
					action_con->start_action(active_win, hover.point, ACTION_CONSTRAINTS_NONE);
				}else{
					get_selected(get_select_mode());
					sel_rect.start_later(m);
				}
			}
		}
	}
	notify_end();
}



void MultiView::on_middle_button_down()
{
	notify_begin();
	active_win = mouse_win;

// move camera?
	if (allow_infinite_scrolling)
		hold_cursor(true);
	view_moving = true;

	notify(MESSAGE_UPDATE);
	notify_end();
}



void MultiView::on_right_button_down()
{
	notify_begin();

	if (hover.meta == hover.HOVER_WINDOW_LABEL){
		active_win = mouse_win;
		menu->open_popup(ed);
	}else{
		active_win = mouse_win;

// move camera?
		if (allow_infinite_scrolling)
			hold_cursor(true);
		view_moving = true;
	}

	notify(MESSAGE_UPDATE);
	notify_end();
}



void MultiView::on_middle_button_up()
{
	notify_begin();
	if (view_moving){
		view_moving = false;
		hold_cursor(false);
	}
	notify_end();
}



void MultiView::on_right_button_up()
{
	notify_begin();
	if (view_moving){
		view_moving = false;
		hold_cursor(false);
	}
	notify_end();
}



void MultiView::on_key_up(int key_code)
{
}



void MultiView::on_left_button_up()
{
	notify_begin();
	end_selection_rect();
	moving_cross_x = false;
	moving_cross_y = false;

	action_con->on_left_button_up();
	cam_con->onLeftButtonUp();
	notify_end();
}



void MultiView::update_mouse()
{
	m.x = hui::GetEvent()->mx;
	m.y = hui::GetEvent()->my;
	m.z = 0;
	v.x = hui::GetEvent()->dx;
	v.y = hui::GetEvent()->dy;
	v.z = 0;

	lbut = hui::GetEvent()->lbut;
	mbut = hui::GetEvent()->mbut;
	rbut = hui::GetEvent()->rbut;

	if (allow_mouse_actions)
		if (cam_con->isMouseOver())
			return;

	// which window is the cursor in?
	for (auto w: win)
		if (w->dest.inside(m.x, m.y))
			mouse_win = w;
	if (!mode3d){
		mouse_win = win[0];
	}
}


void MultiView::on_mouse_move()
{
	notify_begin();
	update_mouse();

	if (action_con->in_use()){
		action_con->on_mouse_move();
	}else if (cam_con->inUse()){
		cam_con->onMouseMove();
	}else if (sel_rect.active and allow_select){
		select_all_in_rectangle(get_select_mode());
	}else if (view_moving){
		int t = active_win->type;
		if ((t == VIEW_PERSPECTIVE) or (t == VIEW_ISOMETRIC)){
	// camera rotation
			cam_rotate_pixel(v, mbut or ed->get_key(hui::KEY_CONTROL));
		}else{
	// camera translation
			cam_move_pixel(v);
		}
	}else if (moving_cross_x or moving_cross_y){
		if (moving_cross_x)
			window_partition_x = clampf((m.x - area.x1) / area.width(), 0.01f, 0.99f);
		if (moving_cross_y)
			window_partition_y = clampf((m.y - area.y1) / area.height(), 0.01f, 0.99f);
	}else if (sel_rect.dist >= 0 and allow_select){
		sel_rect.dist += abs(v.x) + abs(v.y);
		if (sel_rect.dist >= MIN_MOUSE_MOVE_TO_INTERACT)
			start_selection_rect();
	}else{

		get_hover();

	}



	// ignore mouse, while "holding"
	if (holding_cursor){
		if (fabs(m.x - holding_x) + fabs(m.y - holding_y) > 100)
			ed->set_cursor_pos(holding_x, holding_y);
	}

	notify(MESSAGE_UPDATE);
	notify_end();
}




void MultiView::start_selection_rect()
{
	sel_rect.active = true;
	sel_rect.dist = -1;

	// reset selection data
	for (DataSet &d: data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->m_old = sd->is_selected;
			}

	notify(MESSAGE_UPDATE);
}

void MultiView::end_selection_rect()
{
	sel_rect.end();

	notify(MESSAGE_UPDATE);
}


void MultiView::force_redraw() {
	ed->redraw("nix-area");
}


string MultiView::get_unit_by_zoom(vector &v) {
	const char *units[] = {"y", "z", "a", "f", "p", "n", "\u00b5", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y"};
	float l = active_win->get_grid_d() * 10.1f;

	int n = floor(log10(l) / 3.0f);
	v /= exp10(n * 3);
	if ((n >= -8) and  (n <= 8))
		return units[n + 8];
	return format("*10^%d", n*3);
}

string MultiView::format_length(float l) {
	vector v = vector(l, 0, 0);
	string unit = get_unit_by_zoom(v);
	return f2s(v.x,2) + " " + unit;
}

void MultiView::draw_mouse_pos()
{
	vector m = get_cursor();
	string unit = get_unit_by_zoom(m);
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;

	if (mouse_win->type == VIEW_2D){
		draw_str(nix::target_width, nix::target_height - 60, sx + "\n" + sy, TextAlign::RIGHT);
	}else{
		draw_str(nix::target_width, nix::target_height - 80, sx + "\n" + sy +  + "\n" + sz, TextAlign::RIGHT);
	}
}


void MultiView::on_draw()
{
	timer.reset();

	nix::ResetZ();
	nix::SetProjectionOrtho(false);
	nix::SetZ(true,true);
	nix::SetColor(scheme.TEXT);
	set_font(scheme.FONT_NAME, scheme.FONT_SIZE);

	area = nix::target_rect;

	if (!mode3d){
		win[0]->dest = area;
		win[0]->draw();
	}else if (whole_window){
		for (auto w: win)
			w->dest = rect(0,0,0,0);
		active_win->dest = area;
		active_win->draw();
	}else{
		float xm = area.x1 + area.width() * window_partition_x;
		float ym = area.y1 + area.height() * window_partition_y;
		float d = scheme.WINDOW_DIVIDER_THICKNESS / 2;

		// top left
		win[0]->dest = rect(area.x1, xm-d+1, area.y1, ym-d+1);
		win[0]->draw();

		// top right
		win[1]->dest = rect(xm+d-1, area.x2, area.y1, ym-d+1);
		win[1]->draw();

		// bottom left
		win[2]->dest = rect(area.x1, xm-d+1, ym+d-1, area.y2);
		win[2]->draw();

		// bottom right
		win[3]->dest = rect(xm+d-1, area.x2, ym+d-1, area.y2);
		win[3]->draw();

		nix::SetScissor(nix::target_rect);

		nix::SetShader(nix::default_shader_2d);
		nix::SetTexture(NULL);

		color c2 = scheme.hoverify(scheme.WINDOW_DIVIDER);
		nix::SetColor((hover.meta == hover.HOVER_WINDOW_DIVIDER_Y or hover.meta == hover.HOVER_WINDOW_DIVIDER_CENTER) ? c2 : scheme.WINDOW_DIVIDER);
		nix::DrawRect(area.x1, area.x2, ym-d, ym+d, 0);
		nix::SetColor((hover.meta == hover.HOVER_WINDOW_DIVIDER_X or hover.meta == hover.HOVER_WINDOW_DIVIDER_CENTER) ? c2 : scheme.WINDOW_DIVIDER);
		nix::DrawRect(xm-d, xm+d, area.y1, area.y2, 0);
	}

	nix::SetShader(nix::default_shader_2d);
	if (sel_rect.active)
		sel_rect.draw(m);

	cam_con->draw();

	nix::SetColor(scheme.TEXT);

	if (ed->input.inside_smart)
		draw_mouse_pos();

	action_con->draw_post();

	//printf("%f\n", timer.get()*1000.0f);
}

void MultiView::SelectionRect::start_later(const vector &m)
{
	pos0 = m;
	dist = 0;
}

void MultiView::SelectionRect::end()
{
	active = false;
	dist = -1;
}

void MultiView::SelectionRect::draw(const vector &m) {
	nix::SetZ(false, false);
	nix::SetAlphaM(ALPHA_MATERIAL);
	nix::SetTexture(NULL);
	nix::SetColor(scheme.SELECTION_RECT);
	nix::SetCull(CULL_NONE);
	nix::DrawRect(m.x, pos0.x, m.y, pos0.y, 0);
	nix::SetCull(CULL_DEFAULT);
	nix::SetColor(scheme.SELECTION_RECT_BOUNDARY);
	nix::DrawLine(pos0.x, pos0.y, pos0.x, m.y, 0);
	nix::DrawLine(m.x, pos0.y, m.x, m.y, 0);
	nix::DrawLine(pos0.x, pos0.y, m.x, pos0.y, 0);
	nix::DrawLine(pos0.x, m.y, m.x, m.y, 0);
	nix::SetAlphaM(ALPHA_NONE);
	nix::SetZ(true, true);
}

rect MultiView::SelectionRect::get(const vector &m)
{
	return rect(min(m.x, pos0.x), max(m.x, pos0.x), min(m.y, pos0.y), max(m.y, pos0.y));
}

void MultiView::set_mouse_action(const string & name, int mode, bool locked)
{
	if (!mode3d and (mode == ACTION_ROTATE))
		mode = ACTION_ROTATE_2D;
	action_con->action.name = name;
	action_con->action.mode = mode;
	action_con->action.locked = locked;
	action_con->show(need_action_controller());
	notify(MESSAGE_SETTINGS_CHANGE);
}

bool MultiView::need_action_controller()
{
	if (!has_selection())
		return false;
	if (!allow_mouse_actions)
		return false;
	if (action_con->action.mode == ACTION_SELECT)
		return false;
	return true;
}

void MultiView::select_all()
{
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = true;
		}
	action_con->show(need_action_controller());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiView::select_none()
{
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			sd->is_selected = false;
		}
	action_con->show(need_action_controller());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiView::invert_selection()
{
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = !sd->is_selected;
		}
	action_con->show(need_action_controller());
	notify(MESSAGE_SELECTION_CHANGE);
}

bool MultiView::has_selection()
{
	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->is_selected)
				return true;
		}
	return false;
}

vector MultiView::get_selection_center()
{
	vector min = v_0, max = v_0;
	bool first = true;
	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->is_selected){
				if (first){
					min = max = sd->pos;
					first = false;
				}else{
					min._min(sd->pos);
					max._max(sd->pos);
				}
			}
		}
	return (min + max) / 2;
}

vector MultiView::snap_v2(const vector &v, float d) {
	vector w;
	w.x = d * roundf(v.x / d);
	w.y = d * roundf(v.y / d);
	w.z = d * roundf(v.z / d);
	return w;
}

vector MultiView::snap_v(const vector &v) {
	return snap_v2(v, active_win->get_grid_d());
}

float MultiView::snap_f(float f) {
	float d = active_win->get_grid_d();
	return d * roundf(f / d);
}

vector MultiView::maybe_snap_v2(const vector &v, float d) {
	if (snap_to_grid)
		return snap_v2(v, d);
	return v;
}

vector MultiView::maybe_snap_v(const vector &v) {
	if (snap_to_grid)
		return snap_v(v);
	return v;
}

float MultiView::maybe_snap_f(float f) {
	if (snap_to_grid)
		return snap_f(f);
	return f;
}

vector MultiView::get_cursor() {
	if (hover.data)
		return hover.point;
	if (snap_to_grid)
		return snap_v(mouse_win->unproject(m, cam.pos));
	return mouse_win->unproject(m, cam.pos);
}

vector MultiView::get_cursor(const vector &depth_reference) {
	return mouse_win->unproject(m, depth_reference);
}


void MultiView::get_hover()
{
	hover.reset();

	if (!ed->input.inside_smart)
		return;


	/*if (!MVSelectable)
		return;*/
	if (menu and (mouse_win->name_dest.inside(m.x, m.y))){
		hover.meta = hover.HOVER_WINDOW_LABEL;
		return;
	}
	if (cam_con->isMouseOver()){
		hover.meta = hover.HOVER_CAMERA_CONTROLLER;
		return;
	}
	if (!whole_window) {
		float xm = win[0]->dest.x2;
		float ym = win[0]->dest.y2;
		if ((m.x >= xm - 5) and (m.x <= xm + 5) and (m.y >= ym - 5) and (m.y <= ym + 5)) {
			hover.meta = hover.HOVER_WINDOW_DIVIDER_CENTER;
			return;
		} else if ((m.x >= xm - 5) and (m.x <= xm + 5)) {
			hover.meta = hover.HOVER_WINDOW_DIVIDER_X;
			return;
		} else if ((m.y >= ym - 5) and (m.y <= ym + 5)) {
			hover.meta = hover.HOVER_WINDOW_DIVIDER_Y;
			return;
		}
	}
	if (allow_mouse_actions and action_con->is_mouse_over(hover.point)){
		hover.meta = hover.HOVER_ACTION_CONTROLLER;
		return;
	}
	float z_min=1;
	foreachi(DataSet &d, data, di)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d, i);
				if (sd->view_stage < view_stage)
					continue;
				float z;
				vector tp, mop;
				bool mo = false;
				if (d.func_hover)
					mo = d.func_hover(sd, mouse_win, m, tp, z, d.user_data);
				else
					mo = sd->hover(mouse_win, m, tp, z, d.user_data);
				if (mo){
					if (z<z_min){
						z_min = z;
						mop = tp;
					}else{
						if (sd->is_selected){
							mop = tp;
						}else
							continue;
					}
				}
				if (mo){
					hover.index = i;
					hover.set = di;
					hover.type = d.type;
					hover.point = mop;
					hover.meta = hover.HOVER_DATA;
					hover.data = sd;
					if (sd->is_selected)
						return;
				}
			}
}

bool MultiView::hover_selected()
{
	if (hover.index < 0)
		return false;
	return hover.data->is_selected;
}

bool MultiView::has_selectable_data()
{
	for (DataSet &d: data)
		if (d.selectable)
			return true;
	return false;
}

void MultiView::get_selected(int mode)
{
	notify_begin();
	if ((hover.index < 0) or (hover.type < 0)){
		if (mode == SELECT_SET)
			select_none();
	}else{
		SingleData* sd=MVGetSingleData(data[hover.set], hover.index);
		if (sd->is_selected){
			if (mode == SELECT_INVERT){
				sd->is_selected=false;
			}
		}else{
			if (mode == SELECT_SET){
				select_none();
				sd->is_selected=true;
			}else{
				sd->is_selected=true;
			}
		}
	}
	action_con->show(need_action_controller());
	notify(MESSAGE_SELECTION_CHANGE);
	notify_end();
}

void MultiView::select_all_in_rectangle(int mode)
{
	notify_begin();
	// reset data
	select_none();

	rect r = sel_rect.get(m);

	// select
	for (DataSet &d: data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage<view_stage)
					continue;

				// selected?
				sd->m_delta = sd->inRect(active_win, r, d.user_data);

				// add the selection layers
				if (mode == SELECT_INVERT)
					sd->is_selected = (sd->m_old and !sd->m_delta) or (!sd->m_old and sd->m_delta);
				else if (mode == SELECT_ADD)
					sd->is_selected = (sd->m_old or sd->m_delta);
				else
					sd->is_selected = sd->m_delta;
			}

	action_con->show(need_action_controller());
	notify(MESSAGE_SELECTION_CHANGE);
	notify_end();
}

void MultiView::hold_cursor(bool holding)
{
	holding_x = m.x;
	holding_y = m.y;
	holding_cursor = holding;
	ed->show_cursor(!holding);
}



void MultiView::add_message_3d(const string &str, const vector &pos)
{
	Message3d m;
	m.str = str;
	m.pos = pos;
	message3d.add(m);
}

void MultiView::set_allow_action(bool allow)
{
	allow_mouse_actions = allow;
	action_con->show(need_action_controller());
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::set_allow_select(bool allow)
{
	allow_select = allow;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::push_settings()
{
	Settings s;
	s.allow_select = allow_select;
	s.allow_action = allow_mouse_actions;
	s.action_name = action_con->action.name;
	s.action_mode = action_con->action.mode;
	s.action_locked = action_con->action.locked;
	settings_stack.add(s);
}

void MultiView::pop_settings()
{
	Settings s = settings_stack.pop();
	allow_select = s.allow_select;
	allow_mouse_actions = s.allow_action;
	action_con->action.name = s.action_name;
	action_con->action.mode = s.action_mode;
	action_con->action.locked = s.action_locked;
	action_con->show(need_action_controller());
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::view_stage_push()
{
	view_stage ++;

	for (DataSet &d: data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->is_selected)
					sd->view_stage = view_stage;
			}
	notify(MESSAGE_VIEWSTAGE_CHANGE);
}

void MultiView::view_stage_pop()
{
	if (view_stage <= 0)
		return;
	view_stage --;
	for (DataSet &d: data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage > view_stage)
					sd->view_stage = view_stage;
			}
	notify(MESSAGE_VIEWSTAGE_CHANGE);
}

void MultiView::reset_message_3d()
{
	message3d.clear();
}



};
