/*
 * MultiView.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../Edward.h"
#include "MultiView.h"
#include "Window.h"
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

color MultiView::ColorBackGround;
color MultiView::ColorBackGroundSelected;
color MultiView::ColorGrid;
color MultiView::ColorText;
color MultiView::ColorWindowType;
color MultiView::ColorPoint;
color MultiView::ColorPointSelected;
color MultiView::ColorPointSpecial;
color MultiView::ColorWindowSeparator;
color MultiView::ColorSelectionRect;
color MultiView::ColorSelectionRectBoundary;
color MultiView::ColorCreation;
color MultiView::ColorCreationLine;


void MultiView::Selection::reset()
{
	meta = HOVER_NONE;
	index = set = type = -1;
	data = NULL;
}

MultiView::MultiView(bool mode3d) :
	Observable("MultiView")
{
	/*ColorBackGround3D = color(1,0,0,0.15f);
	ColorBackGround2D = color(1,0,0,0.10f);
	ColorGrid = color(1,0.7f,0.7f,0.7f);
	ColorText = White;
	ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	ColorPoint = color(1, 0.2f, 0.2f, 0.9f);
	ColorPointSelected = color(1, 0.9f, 0.2f, 0.2f);
	ColorPointSpecial = color(1, 0.2f, 0.8f, 0.2f);
	ColorWindowSeparator = color(1, 0.1f, 0.1f, 0.6f); // color(1,0.1f,0.1f,0.5f)
	ColorSelectionRect = color(0.2f,0,0,1);
	ColorSelectionRectBoundary = color(0.7f,0,0,1);*/
	ColorBackGround = color(1,0.9f,0.9f,0.9f);
	ColorBackGroundSelected = color(1,0.96f,0.96f,0.96f);
	ColorGrid = color(1,0.5f,0.5f,0.5f);
	ColorText = Black;
	ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	ColorPoint = color(1, 0.2f, 0.2f, 0.9f);
	ColorPointSelected = color(1, 0.9f, 0.2f, 0.2f);
	ColorPointSpecial = color(1, 0.2f, 0.8f, 0.2f);
	ColorWindowSeparator = color(1, 0.1f, 0.1f, 0.6f); // color(1,0.1f,0.1f,0.5f)
	ColorSelectionRect = color(0.2f,0,0,1);
	ColorSelectionRectBoundary = color(0.7f,0,0,1);
	ColorCreation = color(0.5f, 0.1f, 0.6f, 0.1f);
	ColorCreationLine = color(1, 0.1f, 0.8f, 0.1f);

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


	SPEED_MOVE = 20;
	SPEED_ZOOM_KEY = 1.15f;
	SPEED_ZOOM_WHEEL = 1.15f;

	MIN_MOUSE_MOVE_TO_INTERACT = 5;
	MOUSE_ROTATION_SPEED = 0.0033f;

	POINT_RADIUS = 2;
	POINT_RADIUS_HOVER = 4;

	allow_infinite_scrolling = hui::Config.getBool("MultiView.InfiniteScrolling", false);

	if (mode3d){
		win.add(new Window(this, VIEW_BACK));
		win.add(new Window(this, VIEW_LEFT));
		win.add(new Window(this, VIEW_TOP));
		win.add(new Window(this, VIEW_PERSPECTIVE));
		light = 0;

		// Menu
		menu = new hui::Menu;
		menu->addItem(_("Ansicht"), "view_menu_sign");
		//menu->enableItem("view_menu_sign", false);
		menu->addSeparator();
		menu->addItem(_("Rechts (-x)"), "view_right");
		menu->addItem(_("Links (x)"), "view_left");
		menu->addItem(_("Vorne (z)"), "view_front");
		menu->addItem(_("Hinten (-z)"), "view_back");
		menu->addItem(_("Oben (-y)"), "view_top");
		menu->addItem(_("Unten (y)"), "view_bottom");
		menu->addItem(_("Isometrisch"), "view_isometric");
		menu->addItem(_("Perspektive"), "view_perspective");
	}else{
		win.add(new Window(this, VIEW_2D));
		light = -1;
	}
	action_con = new ActionController(this);
	cam_con = new CameraController(this);
	m = v_0;
	holding_cursor = false;
	holding_x = holding_y = 0;
	allow_mouse_actions = true;
	allow_select = true;


	if (!shader_lines_3d)
		shader_lines_3d = nix::LoadShader(app->directory_static + "shader/lines-3d.shader");
	if (!shader_lines_3d_colored)
		shader_lines_3d_colored = nix::LoadShader(app->directory_static + "shader/lines-3d-colored.shader");
	if (!shader_lines_3d_colored_wide)
		shader_lines_3d_colored_wide = nix::LoadShader(app->directory_static + "shader/lines-3d-colored-wide.shader");
	if (!shader_selection)
		shader_selection = nix::LoadShader(app->directory_static + "shader/selection.shader");

	reset();
}

MultiView::~MultiView()
{
	hui::Config.getBool("MultiView.InfiniteScrolling", allow_infinite_scrolling);
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

	clearData(NULL);
	resetMouseAction();
	resetView();
}

void MultiView::resetView()
{
	cam.pos = v_0;
	cam.ang = v_0;
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

	view_stage = 0;

	hover.reset();
	notify(MESSAGE_CAMERA_CHANGE);
	notify(MESSAGE_VIEWSTAGE_CHANGE);
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::resetMouseAction()
{
	action_con->cur_action = NULL;
	action_con->action.reset();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::clearData(Data *_data)
{
	data.clear();
	action_con->data = _data;
	if (!_data)
		resetMouseAction();
}

void MultiView::addData(int type, const DynamicArray & a, void *user_data, int flags)
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

void MultiView::SetViewStage(int *view_stage, bool allow_handle)
{}



void MultiView::camZoom(float factor, bool mouse_rel)
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
void MultiView::camMove(const vector &dir)
{
	vector d = active_win->reflection_matrix * dir;
	cam.pos -= (active_win->local_ang * d) / active_win->zoom();

	notify(MESSAGE_CAMERA_CHANGE);
}

// dir: screen pixels...yap
void MultiView::camRotate(const vector &dir, bool cam_center)
{
	vector dang = vector(dir.y, dir.x, 0) * MOUSE_ROTATION_SPEED;
	// could have used reflection_matrix... but...only VIEW_PERSPECTIVE...

	if (cam_center)
		cam.pos -= cam.radius * (cam.ang * e_z);
	quaternion dq;
	QuaternionRotationV(dq, dang);
	cam.ang = cam.ang * dq;
	if (cam_center)
		cam.pos += cam.radius * (cam.ang * e_z);
	action_con->update();
	notify(MESSAGE_CAMERA_CHANGE);
}

void MultiView::setViewBox(const vector &min, const vector &max)
{
	cam.pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.3f;// * ((float)NixScreenWidth / (float)nix::target_width);
	if (r > 0)
		cam.radius = r;
	notify(MESSAGE_CAMERA_CHANGE);
}

void MultiView::toggleWholeWindow()
{
	whole_window = !whole_window;
	action_con->update();
	notify(MESSAGE_CAMERA_CHANGE);
}

void MultiView::toggleGrid()
{
	grid_enabled = !grid_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::toggleLight()
{
	light_enabled = !light_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::toggleWire()
{
	wire_mode = !wire_mode;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::toggleSnapToGrid()
{
	snap_to_grid = !snap_to_grid;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::onCommand(const string & id)
{
	notifyBegin();

	if (id == "select_all")
		selectAll();
	if (id == "select_none")
		selectNone();
	if (id == "invert_selection")
		invertSelection();

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
		toggleWholeWindow();
	if (id == "grid")
		toggleGrid();
	if (id == "snap_to_grid")
		toggleSnapToGrid();
	if (id == "light")
		toggleLight();
	if (id == "wire")
		toggleWire();

	if (id == "view_push")
		viewStagePush();
	if (id == "view_pop")
		viewStagePop();
	notifyEnd();
}

void MultiView::onMouseWheel()
{
	notifyBegin();
	hui::Event *e = hui::GetEvent();

	// mouse wheel -> zoom
	if (e->scroll_y > 0)
		camZoom(SPEED_ZOOM_WHEEL, mouse_win->type != VIEW_PERSPECTIVE);
	if (e->scroll_y < 0)
		camZoom(1.0f / SPEED_ZOOM_WHEEL, mouse_win->type != VIEW_PERSPECTIVE);
	notifyEnd();
}

void MultiView::onMouseEnter()
{
	notify(MESSAGE_UPDATE);
}

void MultiView::onMouseLeave()
{
	//notify(MESSAGE_UPDATE);
	onMouseMove();
}



void MultiView::onKeyDown(int k)
{
	notifyBegin();

	if ((k == hui::KEY_ADD) or (k == hui::KEY_NUM_ADD))
		camZoom(SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if ((k == hui::KEY_SUBTRACT) or (k == hui::KEY_NUM_SUBTRACT))
		camZoom(1.0f / SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if (k == hui::KEY_RIGHT)
		camMove(-e_x * SPEED_MOVE);
	if (k == hui::KEY_LEFT)
		camMove( e_x * SPEED_MOVE);
	if (k == hui::KEY_UP)
		camMove( e_y * SPEED_MOVE);
	if (k == hui::KEY_DOWN)
		camMove(-e_y * SPEED_MOVE);
	if (k == hui::KEY_SHIFT + hui::KEY_UP)
		camMove( e_z * SPEED_MOVE);
	if (k == hui::KEY_SHIFT + hui::KEY_DOWN)
		camMove(-e_z * SPEED_MOVE);
	if (k == hui::KEY_ESCAPE)
		action_con->endAction(false);
	notifyEnd();
}


int get_select_mode()
{
	if (ed->getKey(hui::KEY_CONTROL))
		return MultiView::SELECT_ADD;
	if (ed->getKey(hui::KEY_SHIFT))
		return MultiView::SELECT_INVERT;
	return MultiView::SELECT_SET;
}

void MultiView::onLeftButtonDown()
{
	notifyBegin();
	updateMouse();

	getHover();

	// menu for selection of view type
	if (hover.meta == hover.HOVER_WINDOW_LABEL){
		active_win = mouse_win;
	}else if (hover.meta == hover.HOVER_CAMERA_CONTROLLER){
		cam_con->onLeftButtonDown();
	}else if (hover.meta == hover.HOVER_ACTION_CONTROLLER){
		active_win = mouse_win;
		action_con->leftButtonDown();
	}else{
		active_win = mouse_win;
		if (action_con->action.locked){
			if (action_con->action.mode == ACTION_SELECT){
				if (allow_select){
					getSelected(get_select_mode());
					sel_rect.start_later(m);
				}
			}else if (allow_mouse_actions and hoverSelected()){
				action_con->startAction(ACTION_CONSTRAINTS_NONE);
			}
		}else{
			if (allow_select){
				if (hoverSelected() and (get_select_mode() == MultiView::SELECT_SET)){
					action_con->startAction(ACTION_CONSTRAINTS_NONE);
				}else{
					getSelected(get_select_mode());
					sel_rect.start_later(m);
				}
			}
		}
	}
	notifyEnd();
}



void MultiView::onMiddleButtonDown()
{
	notifyBegin();
	active_win = mouse_win;

// move camera?
	if (allow_infinite_scrolling)
		holdCursor(true);
	view_moving = true;

	notify(MESSAGE_UPDATE);
	notifyEnd();
}



void MultiView::onRightButtonDown()
{
	notifyBegin();

	if (hover.meta == hover.HOVER_WINDOW_LABEL){
		active_win = mouse_win;
		menu->openPopup(ed, m.x, m.y);
	}else{
		active_win = mouse_win;

// move camera?
		if (allow_infinite_scrolling)
			holdCursor(true);
		view_moving = true;
	}

	notify(MESSAGE_UPDATE);
	notifyEnd();
}



void MultiView::onMiddleButtonUp()
{
	notifyBegin();
	if (view_moving){
		view_moving = false;
		holdCursor(false);
	}
	notifyEnd();
}



void MultiView::onRightButtonUp()
{
	notifyBegin();
	if (view_moving){
		view_moving = false;
		holdCursor(false);
	}
	notifyEnd();
}



void MultiView::onKeyUp(int key_code)
{
}



void MultiView::onLeftButtonUp()
{
	notifyBegin();
	endRect();

	action_con->leftButtonUp();
	cam_con->onLeftButtonUp();
	notifyEnd();
}



void MultiView::updateMouse()
{
	m.x = hui::GetEvent()->mx;
	m.y = hui::GetEvent()->my;
	v.x = hui::GetEvent()->dx;
	v.y = hui::GetEvent()->dy;

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


void MultiView::onMouseMove()
{
	notifyBegin();
	updateMouse();

	if (action_con->inUse()){
		action_con->mouseMove();
	}else if (cam_con->inUse()){
		cam_con->onMouseMove();
	}else if (sel_rect.active and allow_select){
		selectAllInRectangle(get_select_mode());
	}else if (view_moving){
		int t = active_win->type;
		if ((t == VIEW_PERSPECTIVE) or (t == VIEW_ISOMETRIC)){
	// camera rotation
			camRotate(v, mbut or ed->getKey(hui::KEY_CONTROL));
		}else{
	// camera translation
			camMove(v);
		}
	}else if (sel_rect.dist >= 0 and allow_select){
		sel_rect.dist += abs(v.x) + abs(v.y);
		if (sel_rect.dist >= MIN_MOUSE_MOVE_TO_INTERACT)
			startRect();
	}else{

		getHover();

	}



	// ignore mouse, while "holding"
	if (holding_cursor){
		if (fabs(m.x - holding_x) + fabs(m.y - holding_y) > 100)
			ed->setCursorPos(holding_x, holding_y);
	}

	notify(MESSAGE_UPDATE);
	notifyEnd();
}




void MultiView::startRect()
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

void MultiView::endRect()
{
	sel_rect.end();

	notify(MESSAGE_UPDATE);
}



string MultiView::getScaleByZoom(vector &v)
{
	const char *units[] = {"y", "z", "a", "f", "p", "n", "\u00b5", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y"};
	float l = active_win->get_grid_d() * 10.1f;

	int n = floor(log10(l) / 3.0f);
	v /= exp10(n * 3);
	if ((n >= -8) and  (n <= 8))
		return units[n + 8];
	return format("*10^%d", n*3);
}

string format_length(MultiView *mv, float l)
{
	vector v = vector(l, 0, 0);
	string unit = mv->getScaleByZoom(v);
	return f2s(v.x,2) + " " + unit;
}

void MultiView::drawMousePos()
{
	vector m = getCursor3d();
	string unit = getScaleByZoom(m);
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;

	if (mouse_win->type == VIEW_2D){
		ed->drawStr(nix::target_width, nix::target_height - 60, sx, Edward::ALIGN_RIGHT);
		ed->drawStr(nix::target_width, nix::target_height - 40, sy, Edward::ALIGN_RIGHT);
	}else{
		ed->drawStr(nix::target_width, nix::target_height - 80, sx, Edward::ALIGN_RIGHT);
		ed->drawStr(nix::target_width, nix::target_height - 60, sy, Edward::ALIGN_RIGHT);
		ed->drawStr(nix::target_width, nix::target_height - 40, sz, Edward::ALIGN_RIGHT);
	}
}


void MultiView::onDraw()
{
	timer.reset();

	nix::ResetZ();
	nix::SetProjectionOrtho(false);
	nix::SetZ(true,true);
	nix::SetColor(ColorText);


	if (!mode3d){
		win[0]->dest = nix::target_rect;
		win[0]->draw();
	}else if (whole_window){
		for (auto w: win)
			w->dest = rect(0,0,0,0);
		active_win->dest = nix::target_rect;
		active_win->draw();
	}else{
		// top left
		win[0]->dest = rect(0,nix::target_width/2,0,nix::target_height/2);
		win[0]->draw();

		// top right
		win[1]->dest = rect(nix::target_width/2,nix::target_width,0,nix::target_height/2);
		win[1]->draw();

		// bottom left
		win[2]->dest = rect(0,nix::target_width/2,nix::target_height/2,nix::target_height);
		win[2]->draw();

		// bottom right
		win[3]->dest = rect(nix::target_width/2,nix::target_width,nix::target_height/2,nix::target_height);
		win[3]->draw();

		nix::Scissor(nix::target_rect);
		nix::EnableLighting(false);

		nix::SetShader(nix::default_shader_2d);
		nix::SetTexture(NULL);
		nix::SetColor(ColorWindowSeparator);
		nix::DrawRect(0, nix::target_width, nix::target_height/2-1, nix::target_height/2+2, 0);
		nix::DrawRect(nix::target_width/2-1, nix::target_width/2+2, 0, nix::target_height, 0);
	}
	nix::EnableLighting(false);

	nix::SetShader(nix::default_shader_2d);
	if (sel_rect.active)
		sel_rect.draw(m);

	cam_con->draw();

	nix::SetColor(ColorText);

	if (ed->input.inside_smart)
		drawMousePos();

	action_con->drawPost();

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

void MultiView::SelectionRect::draw(const vector &m)
{
	nix::SetZ(false, false);
	nix::SetAlphaM(ALPHA_MATERIAL);
	nix::SetTexture(NULL);
	nix::SetColor(ColorSelectionRect);
	nix::SetCull(CULL_NONE);
	nix::DrawRect(m.x, pos0.x, m.y, pos0.y, 0);
	nix::SetCull(CULL_DEFAULT);
	nix::SetColor(ColorSelectionRectBoundary);
	nix::DrawLineV(pos0.x	,pos0.y	,m.y	,0);
	nix::DrawLineV(m.x	,pos0.y	,m.y	,0);
	nix::DrawLineH(pos0.x	,m.x	,pos0.y	,0);
	nix::DrawLineH(pos0.x	,m.x	,m.y	,0);
	nix::SetAlphaM(ALPHA_NONE);
	nix::SetZ(true, true);
}

rect MultiView::SelectionRect::get(const vector &m)
{
	return rect(min(m.x, pos0.x), max(m.x, pos0.x), min(m.y, pos0.y), max(m.y, pos0.y));
}

void MultiView::setMouseAction(const string & name, int mode, bool locked)
{
	if (!mode3d and (mode == ACTION_ROTATE))
		mode = ACTION_ROTATE_2D;
	action_con->action.name = name;
	action_con->action.mode = mode;
	action_con->action.locked = locked;
	action_con->show(needActionController());
	notify(MESSAGE_SETTINGS_CHANGE);
}

bool MultiView::needActionController()
{
	if (!hasSelection())
		return false;
	if (!allow_mouse_actions)
		return false;
	if (action_con->action.mode == ACTION_SELECT)
		return false;
	return true;
}

void MultiView::selectAll()
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
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiView::selectNone()
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
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiView::invertSelection()
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
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

bool MultiView::hasSelection()
{
	for (DataSet &d: data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->is_selected)
				return true;
		}
	return false;
}

vector MultiView::getSelectionCenter()
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

vector snap_v2(const vector &v, float d)
{
	vector w;
	w.x = d * roundf(v.x / d);
	w.y = d * roundf(v.y / d);
	w.z = d * roundf(v.z / d);
	return w;
}

vector snap_v(MultiView *mv, const vector &v)
{
	return snap_v2(v, mv->active_win->get_grid_d());
}

float snap_f(MultiView *mv, float f)
{
	float d = mv->active_win->get_grid_d();
	return d * roundf(f / d);
}

vector MultiView::getCursor3d()
{
	if (hover.data)
		return hover.point;
	if (snap_to_grid)
		return snap_v(this, mouse_win->unproject(m, cam.pos));
	return mouse_win->unproject(m, cam.pos);
}

vector MultiView::getCursor3d(const vector &depth_reference)
{
	return mouse_win->unproject(m, depth_reference);
}


void MultiView::getHover()
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
	if (allow_mouse_actions and action_con->isMouseOver(hover.point)){
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
				bool mo = sd->hover(mouse_win, m, tp, z, d.user_data);
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

bool MultiView::hoverSelected()
{
	if (hover.index < 0)
		return false;
	return hover.data->is_selected;
}

bool MultiView::hasSelectableData()
{
	for (DataSet &d: data)
		if (d.selectable)
			return true;
	return false;
}

void MultiView::unselectAll()
{
	for (DataSet &d: data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->is_selected = false;
			}
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiView::getSelected(int mode)
{
	notifyBegin();
	if ((hover.index < 0) or (hover.type < 0)){
		if (mode == SELECT_SET)
			unselectAll();
	}else{
		SingleData* sd=MVGetSingleData(data[hover.set], hover.index);
		if (sd->is_selected){
			if (mode == SELECT_INVERT){
				sd->is_selected=false;
			}
		}else{
			if (mode == SELECT_SET){
				unselectAll();
				sd->is_selected=true;
			}else{
				sd->is_selected=true;
			}
		}
	}
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
	notifyEnd();
}

void MultiView::selectAllInRectangle(int mode)
{
	notifyBegin();
	// reset data
	unselectAll();

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

	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
	notifyEnd();
}

void MultiView::holdCursor(bool holding)
{
	holding_x = m.x;
	holding_y = m.y;
	holding_cursor = holding;
	ed->showCursor(!holding);
}



void MultiView::addMessage3d(const string &str, const vector &pos)
{
	Message3d m;
	m.str = str;
	m.pos = pos;
	message3d.add(m);
}

void MultiView::setAllowAction(bool allow)
{
	allow_mouse_actions = allow;
	action_con->show(needActionController());
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::setAllowSelect(bool allow)
{
	allow_select = allow;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::pushSettings()
{
	Settings s;
	s.allow_select = allow_select;
	s.allow_action = allow_mouse_actions;
	s.action_name = action_con->action.name;
	s.action_mode = action_con->action.mode;
	s.action_locked = action_con->action.locked;
	settings_stack.add(s);
}

void MultiView::popSettings()
{
	Settings s = settings_stack.pop();
	allow_select = s.allow_select;
	allow_mouse_actions = s.allow_action;
	action_con->action.name = s.action_name;
	action_con->action.mode = s.action_mode;
	action_con->action.locked = s.action_locked;
	action_con->show(needActionController());
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiView::viewStagePush()
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

void MultiView::viewStagePop()
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

void MultiView::resetMessage3d()
{
	message3d.clear();
}

void set_wide_lines(float width)
{
	if (width == 1.0f){
		nix::SetShader(shader_lines_3d_colored);
	}else{
		auto s = shader_lines_3d_colored_wide;
		nix::SetShader(s);
		int loc_tw = s->get_location("target_width");
		int loc_th = s->get_location("target_height");
		int loc_lw = s->get_location("line_width");
		s->set_float(loc_tw, nix::target_width);
		s->set_float(loc_th, nix::target_height);
		s->set_float(loc_lw, width);
	}
}


};
