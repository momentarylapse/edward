/*
 * MultiViewImpl.cpp
 *
 *  Created on: 22.01.2014
 *      Author: michi
 */

#include "MultiViewImpl.h"

#include "../Edward.h"
#include "MultiView.h"
#include "Window.h"
#include "ActionController.h"
#include "CameraController.h"
#include "SingleData.h"

namespace MultiView{


#define update_zoom		\
	if (mode3d) \
		cam.zoom = ((float)NixScreenHeight / (whole_window ? 1.0f : 2.0f) / cam.radius); \
	else \
		cam.zoom = (float)NixScreenHeight * 0.8f / cam.radius;
#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))
//#define MVGetSingleData(d, index)	( dynamic_cast<MultiViewSingleData*> ((char*)(d).data + (d).DataSingleSize * index))


MultiViewImpl::MultiViewImpl(bool _mode3d) :
	MultiView(_mode3d)
{

	SPEED_MOVE = 20;
	SPEED_ZOOM_KEY = 1.15f;
	SPEED_ZOOM_WHEEL = 1.15f;

	MIN_MOUSE_MOVE_TO_INTERACT = 5;
	MOUSE_ROTATION_SPEED = 0.0033f;

	POINT_RADIUS = 2;
	POINT_RADIUS_HOVER = 4;

	allow_infinite_scrolling = HuiConfig.getBool("MultiView.InfiniteScrolling", true);

	mode3d = _mode3d;
	win[0] = new Window(this, VIEW_BACK);
	win[1] = new Window(this, VIEW_LEFT);
	win[2] = new Window(this, VIEW_TOP);
	win[3] = new Window(this, VIEW_PERSPECTIVE);

	if (mode3d){
		light = 0;

		// Menu
		menu = new HuiMenu;
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
		win[0]->type = VIEW_2D;
		light = -1;
	}
	action_con = new ActionController(this);
	cam_con = new CameraController(this);
	m = v_0;
	holding_cursor = false;
	holding_x = holding_y = 0;
	allow_mouse_actions = true;
	allow_select = true;

	reset();
}

MultiViewImpl::~MultiViewImpl()
{
	HuiConfig.getBool("MultiView.InfiniteScrolling", allow_infinite_scrolling);
	for (int i=0;i<4;i++)
		delete(win[i]);
	delete(cam_con);
	delete(action_con);
}

void MultiViewImpl::reset()
{
	sel_rect.end();
	if (mode3d){
		mouse_win = win[3];
		active_win = win[3];
	}else{
		mouse_win = win[0];
		active_win = win[0];
	}

	view_moving = false;

	hover.reset();
	action_con->reset();

	clearData(NULL);
	resetMouseAction();
	resetView();
}

void MultiViewImpl::resetView()
{
	cam.pos = v_0;
	cam.ang = v_0;
	if (mode3d)
		cam.radius = 100;
	else
		cam.radius = 1;
	cam.zoom = 1;
	whole_window = false;
	grid_enabled = true;
	light_enabled = true;
	cam.ignore_radius = false;
	wire_mode = false;

	view_stage = 0;

	hover.reset();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::resetMouseAction()
{
	action_con->cur_action = NULL;
	action_con->action.reset();
	msg_write("reset");
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::clearData(Data *_data)
{
	data.clear();
	action_con->data = _data;
	if (!_data)
		resetMouseAction();
}

void MultiViewImpl::addData(int type, const DynamicArray & a, void *user_data, int flags)
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

void MultiViewImpl::SetViewStage(int *view_stage, bool allow_handle)
{}



void MultiViewImpl::camZoom(float factor, bool mouse_rel)
{
	vector mup;
	if (mouse_rel)
		mup = mouse_win->unproject(m);
	cam.radius /= factor;
	update_zoom;
	if (mouse_rel)
		cam.pos += mup - mouse_win->unproject(m);
	action_con->update();
	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::camMove(const vector &dir)
{
	vector r = active_win->getDirectionRight();
	vector u = active_win->getDirectionUp();
	cam.pos += dir.x / cam.zoom * r + dir.y / cam.zoom * u;
	/*vector d, u, r;
	mouse_win->GetMovingFrame(d, u, r);
	if (mode3d)
		cam.pos += cam.radius*(r*dir.x+u*dir.y+d*dir.z) * SPEED_MOVE;
	else
		cam.pos += (float)NixScreenHeight / cam.zoom*(r*dir.x+u*dir.y) * SPEED_MOVE;*/
	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::camRotate(const vector &dir, bool cam_center)
{
	if (cam_center)
		cam.pos -= cam.radius * (cam.ang * e_z);
	quaternion dang;
	QuaternionRotationV(dang, vector(v.y, v.x, 0) * MOUSE_ROTATION_SPEED);
	cam.ang = cam.ang * dang;
	if (cam_center)
		cam.pos += cam.radius * (cam.ang * e_z);
}

void MultiViewImpl::setViewBox(const vector &min, const vector &max)
{
	cam.pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.3f * ((float)NixScreenWidth / (float)NixTargetWidth);
	if (r > 0)
		cam.radius = r;
	update_zoom;
	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::toggleWholeWindow()
{
	whole_window = !whole_window;
	action_con->update();
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::toggleGrid()
{
	grid_enabled = !grid_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::toggleLight()
{
	light_enabled = !light_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::toggleWire()
{
	wire_mode = !wire_mode;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::onCommand(const string & id)
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

void MultiViewImpl::onMouseWheel()
{
	notifyBegin();
	HuiEvent *e = HuiGetEvent();

	// mouse wheel -> zoom
	if (e->dz > 0)
		camZoom(SPEED_ZOOM_WHEEL, mouse_win->type != VIEW_PERSPECTIVE);
	if (e->dz < 0)
		camZoom(1.0f / SPEED_ZOOM_WHEEL, mouse_win->type != VIEW_PERSPECTIVE);
	notifyEnd();
}

void MultiViewImpl::onMouseEnter()
{
	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::onMouseLeave()
{
	//notify(MESSAGE_UPDATE);
	onMouseMove();
}



void MultiViewImpl::onKeyDown()
{
	notifyBegin();
	int k = HuiGetEvent()->key_code;

	if ((k == KEY_ADD) ||(k == KEY_NUM_ADD))
		camZoom(SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if ((k == KEY_SUBTRACT) || (k == KEY_NUM_SUBTRACT))
		camZoom(1.0f / SPEED_ZOOM_KEY, mouse_win->type != VIEW_PERSPECTIVE);
	if (k == KEY_RIGHT)
		camMove(-e_x * SPEED_MOVE);
	if (k == KEY_LEFT)
		camMove( e_x * SPEED_MOVE);
	if (k == KEY_UP)
		camMove( e_y * SPEED_MOVE);
	if (k == KEY_DOWN)
		camMove(-e_y * SPEED_MOVE);
	if (k == KEY_SHIFT + KEY_UP)
		camMove( e_z * SPEED_MOVE);
	if (k == KEY_SHIFT + KEY_DOWN)
		camMove(-e_z * SPEED_MOVE);
	if (k == KEY_ESCAPE)
		action_con->endAction(false);
	notifyEnd();
}


int get_select_mode()
{
	if (NixGetKey(KEY_CONTROL))
		return MultiViewImpl::SELECT_ADD;
	if (NixGetKey(KEY_SHIFT))
		return MultiViewImpl::SELECT_INVERT;
	return MultiViewImpl::SELECT_SET;
}

void MultiViewImpl::onLeftButtonDown()
{
	notifyBegin();
	updateMouse();

	getHover();

	// menu for selection of view type
	if (hover.meta == hover.HOVER_WINDOW_LABEL){
		active_win = mouse_win;
		menu->openPopup(ed, m.x, m.y);
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
					if (hoverSelected()){
						getSelected(get_select_mode());
						sel_rect.start_later(m);
					}else{
						getSelected(get_select_mode());
						sel_rect.start_later(m);
					}
				}
			}else if ((allow_mouse_actions) && (hoverSelected())){
				action_con->startAction();
			}
		}else{
			if (hasSelectableData()){
				if (hover.data){
					if (hoverSelected()){
						action_con->startAction();
					}else{
						getSelected(get_select_mode());
						sel_rect.start_later(m);
					}
				}else{
					getSelected(get_select_mode());
					sel_rect.start_later(m);
				}
			}
		}
	/*sel_rect.start_later(m);

	//v = v_0;
	if (allow_mouse_actions){
		if (action_con->isSelecting()){
			getSelected(get_select_mode());
		}else if (action_con->leftButtonDown()){
		}
	}*/
	}
	notifyEnd();
}



void MultiViewImpl::onMiddleButtonDown()
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



void MultiViewImpl::onRightButtonDown()
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



void MultiViewImpl::onMiddleButtonUp()
{
	notifyBegin();
	if (view_moving){
		view_moving = false;
		holdCursor(false);
	}
	notifyEnd();
}



void MultiViewImpl::onRightButtonUp()
{
	notifyBegin();
	if (view_moving){
		view_moving = false;
		holdCursor(false);
	}
	notifyEnd();
}



void MultiViewImpl::onKeyUp()
{
}



void MultiViewImpl::onLeftButtonUp()
{
	notifyBegin();
	endRect();

	action_con->leftButtonUp();
	cam_con->onLeftButtonUp();
	notifyEnd();
}



void MultiViewImpl::updateMouse()
{
	m.x = HuiGetEvent()->mx;
	m.y = HuiGetEvent()->my;
	v.x = HuiGetEvent()->dx;
	v.y = HuiGetEvent()->dy;

	lbut = HuiGetEvent()->lbut;
	mbut = HuiGetEvent()->mbut;
	rbut = HuiGetEvent()->rbut;

	if (cam_con->isMouseOver())
		return;

	// which window is the cursor in?
	if (mode3d){
		if (whole_window){
			mouse_win = active_win;
		}else{
			if ((m.x<MaxX/2)&&(m.y<MaxY/2))
				mouse_win = win[0];
			if ((m.x>MaxX/2)&&(m.y<MaxY/2))
				mouse_win = win[1];
			if ((m.x<MaxX/2)&&(m.y>MaxY/2))
				mouse_win = win[2];
			if ((m.x>MaxX/2)&&(m.y>MaxY/2))
				mouse_win = win[3];
		}
	}else{
		mouse_win = win[0];
	}
}


void MultiViewImpl::onMouseMove()
{
	notifyBegin();
	updateMouse();


	if (action_con->inUse()){
		action_con->mouseMove();
	}else if (cam_con->inUse()){
		cam_con->onMouseMove();
	}else if (sel_rect.active){
		selectAllInRectangle(get_select_mode());
	}else if (view_moving){
		int t = active_win->type;
		if ((t == VIEW_PERSPECTIVE) || (t == VIEW_ISOMETRIC)){
	// camera rotation
			camRotate(v, mbut || (NixGetKey(KEY_CONTROL)));
		}else{
	// camera translation
			camMove(v);
		}
	}else if (sel_rect.dist >= 0){
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




void MultiViewImpl::startRect()
{
	sel_rect.active = true;
	sel_rect.dist = -1;

	// reset selection data
	foreach(DataSet &d, data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->m_old = sd->is_selected;
			}

	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::endRect()
{
	sel_rect.end();

	notify(MESSAGE_UPDATE);
}


#define GridConst	5.0f

float MultiViewImpl::getGridD()
{
	float z = cam.zoom,d=1.0f;
	if (z<GridConst){
		for (int i=0;i<40;i++){
			d*=10.0f;
			if (d*z>GridConst)
				break;
		}
	}else if (z>=GridConst*10.0f){
		for (int i=0;i<40;i++){
			d/=10.0f;
			if (d*z<=GridConst*10.0f)
				break;
		}
	}
	return d;
}


string MultiViewImpl::getMVScaleByZoom(vector &v)
{

	float l = getGridD() * 10.1f;
	string unit;
	float f = 1.0f;


	/*float z=Zoom3D,d=1.0f;
	if (z<GridConst){
		for (int i=0;i<40;i++){
			d*=10.0f;
			if (d*z>GridConst)
				break;
		}
	}else if (z>=GridConst*10.0f){
		for (int i=0;i<40;i++){
			d/=10.0f;
			if (d*z<=GridConst*10.0f)
				break;
		}
	}
	return d;*/

	//if (l<=0.0000000000000001f){
	//}
	unit = "???";
	if (l>0.000000000000000001f){	unit = "a";	f=1000000000000000000.0f;	}
	if (l>0.000000000000001f){		unit = "f";	f=1000000000000000.0f;	}
	if (l>0.000000000001f){			unit = "p";	f=1000000000000.0f;	}
	if (l>0.000000001f){			unit = "n";	f=1000000000.0f;	}
	if (l>0.000001f){				unit = "\u00b5";	f=1000000.0f;	}
	if (l>0.001f){					unit = "m";	f=1000.0f;	}
	if (l>1.0f){					unit = "";	f=1.0f;	}
	if (l>1000.0f){					unit = "k";	f=0.001f;	}
	if (l>1000000.0f){				unit = "M";	f=0.000001f;	}
	if (l>1000000000.0f){			unit = "G";	f=0.000000001f;	}
	if (l>1000000000000.0f){		unit = "T";	f=0.000000000001f;	}
	if (l>1000000000000000.0f){		unit = "P";	f=0.000000000000001f;	}
	//if (l>1000000000000000000.0f){
	//}
	v *= f;
	return unit;
}

void MultiViewImpl::drawMousePos()
{
	vector m = getCursor3d();
	string unit = getMVScaleByZoom(m);
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;

	if (mouse_win->type == VIEW_2D){
		ed->drawStr(MaxX, MaxY - 60, sx, Edward::ALIGN_RIGHT);
		ed->drawStr(MaxX, MaxY - 40, sy, Edward::ALIGN_RIGHT);
	}else{
		ed->drawStr(MaxX, MaxY - 80, sx, Edward::ALIGN_RIGHT);
		ed->drawStr(MaxX, MaxY - 60, sy, Edward::ALIGN_RIGHT);
		ed->drawStr(MaxX, MaxY - 40, sz, Edward::ALIGN_RIGHT);
	}
}


void MultiViewImpl::onDraw()
{
	msg_db_f("Multiview.OnDraw",2);

	update_zoom;

	NixSetZ(true,true);
	NixSetColor(ColorText);


	if (!mode3d){
		win[0]->dest = rect(0,MaxX,0,MaxY);
		win[0]->draw();
	}else if (whole_window){
		active_win->dest = rect(0,MaxX,0,MaxY);
		active_win->draw();
	}else{
		// top left
		win[0]->dest = rect(0,MaxX/2,0,MaxY/2);
		win[0]->draw();

		// top right
		win[1]->dest = rect(MaxX/2,MaxX,0,MaxY/2);
		win[1]->draw();

		// bottom left
		win[2]->dest = rect(0,MaxX/2,MaxY/2,MaxY);
		win[2]->draw();

		// bottom right
		win[3]->dest = rect(MaxX/2,MaxX,MaxY/2,MaxY);
		win[3]->draw();

		NixScissor(NixTargetRect);
		NixEnableLighting(false);
		NixSetColor(ColorWindowSeparator);
		NixDrawRect(0, MaxX, MaxY/2-1, MaxY/2+2, 0);
		NixDrawRect(MaxX/2-1, MaxX/2+2, 0, MaxY, 0);
	}
	cur_projection_win = NULL;
	NixEnableLighting(false);

	if (sel_rect.active)
		sel_rect.draw(m);

	cam_con->draw();

	NixSetColor(ColorText);

	if (ed->input.inside_smart)
		drawMousePos();

	if (action_con->inUse())
		action_con->drawParams();
}

void MultiViewImpl::SelectionRect::start_later(const vector &m)
{
	pos0 = m;
	dist = 0;
}

void MultiViewImpl::SelectionRect::end()
{
	active = false;
	dist = -1;
}

void MultiViewImpl::SelectionRect::draw(const vector &m)
{
	NixSetZ(false, false);
	NixSetAlphaM(AlphaMaterial);
	NixSetColor(ColorSelectionRect);
	NixDrawRect(m.x, pos0.x, m.y, pos0.y, 0);
	NixSetColor(ColorSelectionRectBoundary);
	NixDrawLineV(pos0.x	,pos0.y	,m.y	,0);
	NixDrawLineV(m.x	,pos0.y	,m.y	,0);
	NixDrawLineH(pos0.x	,m.x	,pos0.y	,0);
	NixDrawLineH(pos0.x	,m.x	,m.y	,0);
	NixSetAlphaM(AlphaNone);
	NixSetZ(true, true);
}

rect MultiViewImpl::SelectionRect::get(const vector &m)
{
	return rect(min(m.x, pos0.x), max(m.x, pos0.x), min(m.y, pos0.y), max(m.y, pos0.y));
}

void MultiViewImpl::setMouseAction(const string & name, int mode, bool locked)
{
	if ((!mode3d) && (mode == ACTION_ROTATE))
		mode = ACTION_ROTATE_2D;
	action_con->action.name = name;
	action_con->action.mode = mode;
	action_con->action.locked = locked;
	action_con->show(needActionController());
	notify(MESSAGE_SETTINGS_CHANGE);
}

bool MultiViewImpl::needActionController()
{
	if (!hasSelection())
		return false;
	if (action_con->action.mode == ACTION_SELECT)
		return false;
	return true;
}

void MultiViewImpl::selectAll()
{
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = true;
		}
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiViewImpl::selectNone()
{
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			sd->is_selected = false;
		}
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiViewImpl::invertSelection()
{
	if (!allow_select)
		return;
	if (action_con->action.locked)
		if (action_con->action.mode != ACTION_SELECT)
			return;

	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = !sd->is_selected;
		}
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

bool MultiViewImpl::hasSelection()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->is_selected)
				return true;
		}
	return false;
}

vector MultiViewImpl::getSelectionCenter()
{
	vector min = v_0, max = v_0;
	bool first = true;
	foreach(DataSet &d, data)
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

vector MultiViewImpl::getCursor3d()
{
	return mouse_win->unproject(m, cam.pos);
}

vector MultiViewImpl::getCursor3d(const vector &depth_reference)
{
	return mouse_win->unproject(m, depth_reference);
}


void MultiViewImpl::getHover()
{
	msg_db_f("GetMouseOver",6);
	hover.reset();

	if (!ed->input.inside_smart)
		return;

	/*if (!MVSelectable)
		return;*/
	if ((menu) && (mouse_win->name_dest.inside(m.x, m.y))){
		hover.meta = hover.HOVER_WINDOW_LABEL;
		return;
	}
	if (cam_con->isMouseOver()){
		hover.meta = hover.HOVER_CAMERA_CONTROLLER;
		return;
	}
	if (action_con->isMouseOver(hover.point)){
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

bool MultiViewImpl::hoverSelected()
{
	if (hover.index < 0)
		return false;
	return hover.data->is_selected;
}

bool MultiViewImpl::hasSelectableData()
{
	foreach(DataSet &d, data)
		if (d.selectable)
			return true;
	return false;
}

void MultiViewImpl::unselectAll()
{
	foreach(DataSet &d, data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->is_selected = false;
			}
	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiViewImpl::getSelected(int mode)
{
	msg_db_f("GetSelected",4);
	notifyBegin();
	if ((hover.index < 0) || (hover.type < 0)){
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

void MultiViewImpl::selectAllInRectangle(int mode)
{
	msg_db_f("SelAllInRect",4);
	notifyBegin();
	// reset data
	unselectAll();

	rect r = sel_rect.get(m);

	// select
	foreach(DataSet &d, data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage<view_stage)
					continue;

				// selected?
				sd->m_delta = sd->inRect(active_win, r, d.user_data);

				// add the selection layers
				if (mode == SELECT_INVERT)
					sd->is_selected = (sd->m_old && !sd->m_delta) || (!sd->m_old && sd->m_delta);
				else if (mode == SELECT_ADD)
					sd->is_selected = (sd->m_old || sd->m_delta);
				else
					sd->is_selected = sd->m_delta;
			}

	action_con->show(needActionController());
	notify(MESSAGE_SELECTION_CHANGE);
	notifyEnd();
}

void MultiViewImpl::holdCursor(bool holding)
{
	holding_x = m.x;
	holding_y = m.y;
	holding_cursor = holding;
	ed->showCursor(!holding);
}



void MultiViewImpl::addMessage3d(const string &str, const vector &pos)
{
	Message3d m;
	m.str = str;
	m.pos = pos;
	message3d.add(m);
}

void MultiViewImpl::setAllowAction(bool allow)
{
	allow_mouse_actions = allow;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::setAllowSelect(bool allow)
{
	allow_select = allow;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::viewStagePush()
{
	view_stage ++;

	foreach(DataSet &d, data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->is_selected)
					sd->view_stage = view_stage;
			}
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::viewStagePop()
{
	if (view_stage <= 0)
		return;
	view_stage --;
	foreach(DataSet &d, data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage > view_stage)
					sd->view_stage = view_stage;
			}
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::resetMessage3d()
{
	message3d.clear();
}

};
