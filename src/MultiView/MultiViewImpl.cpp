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

const float SPEED_MOVE = 20;
const float SPEED_ZOOM_KEY = 1.15f;
const float SPEED_ZOOM_WHEEL = 1.15f;

const int MinMouseMoveToInteract = 5;
const float MouseRotationSpeed = 0.0033f;


color ColorBackGround3D;
color ColorBackGround2D;
color ColorGrid;
color ColorText;
color ColorWindowType;
color ColorPoint;
color ColorPointSelected;
color ColorPointSpecial;
color ColorWindowSeparator;
color ColorSelectionRect;
color ColorSelectionRectBoundary;

int PointRadius;
int PointRadiusMouseOver;

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
	ColorBackGround3D = color(1,0.9f,0.9f,0.9f);
	ColorBackGround2D = color(1,0.9f,0.9f,0.9f);
	ColorGrid = color(1,0.5f,0.5f,0.5f);
	ColorText = Black;
	ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	ColorPoint = color(1, 0.2f, 0.2f, 0.9f);
	ColorPointSelected = color(1, 0.9f, 0.2f, 0.2f);
	ColorPointSpecial = color(1, 0.2f, 0.8f, 0.2f);
	ColorWindowSeparator = color(1, 0.1f, 0.1f, 0.6f); // color(1,0.1f,0.1f,0.5f)
	ColorSelectionRect = color(0.2f,0,0,1);
	ColorSelectionRectBoundary = color(0.7f,0,0,1);

	PointRadius = 2;
	PointRadiusMouseOver = 4;

	allow_infinite_scrolling = HuiConfig.getBool("MultiView.InfiniteScrolling", true);

	mode3d = _mode3d;
	win[0] = new Window(this, ViewBack);
	win[1] = new Window(this, ViewLeft);
	win[2] = new Window(this, ViewTop);
	win[3] = new Window(this, ViewPerspective);

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
		win[0]->type = View2D;
		light = -1;
	}
	action_con = new ActionController(this);
	cam_con = new CameraController(this);
	m = v_0;
	HoldingCursor = false;
	HoldingX = HoldingY = 0;
	allow_mouse_actions = true;

	Reset();
}

MultiViewImpl::~MultiViewImpl()
{
	HuiConfig.getBool("MultiView.InfiniteScrolling", allow_infinite_scrolling);
	for (int i=0;i<4;i++)
		delete(win[i]);
	delete(cam_con);
	delete(action_con);
}

void MultiViewImpl::Reset()
{
	allow_rect = false;
	sel_rect.active = false;
	if (mode3d){
		mouse_win = win[3];
		active_win = win[3];
	}else{
		mouse_win = win[0];
		active_win = win[0];
	}

	ViewMoving = false;

	hover.reset();
	action_con->reset();

	ClearData(NULL);
	ResetMouseAction();
	ResetView();
}

void MultiViewImpl::ResetView()
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

void MultiViewImpl::ResetMouseAction()
{
	action_con->cur_action = NULL;
	action_con->action.reset();
}

void MultiViewImpl::ClearData(Data *_data)
{
	data.clear();
	action_con->data = _data;
	if (!_data)
		ResetMouseAction();
}

void MultiViewImpl::AddData(int type, const DynamicArray & a, void *user_data, int flags)
{
	DataSet d;
	d.type = type;
	d.data = (DynamicArray*)&a;
	d.user_data = user_data;
	d.selectable = (flags & FlagSelect)>0;
	d.drawable = (flags & FlagDraw)>0;
	d.indexable = (flags & FlagIndex)>0;
	d.movable = (flags & FlagMove)>0;
	data.add(d);
}

void MultiViewImpl::SetViewStage(int *view_stage, bool allow_handle)
{}



void MultiViewImpl::CamZoom(float factor, bool mouse_rel)
{
	vector mup;
	if (mouse_rel)
		mup = mouse_win->unproject(m);
	cam.radius /= factor;
	update_zoom;
	if (mouse_rel)
			cam.pos += mup - mouse_win->unproject(m);
	if (action_con->show)
		action_con->update();
	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::CamMove(const vector &dir)
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

void MultiViewImpl::CamRotate(const vector &dir, bool cam_center)
{
	if (cam_center)
		cam.pos -= cam.radius * (cam.ang * e_z);
	quaternion dang;
	QuaternionRotationV(dang, vector(v.y, v.x, 0) * MouseRotationSpeed);
	cam.ang = cam.ang * dang;
	if (cam_center)
		cam.pos += cam.radius * (cam.ang * e_z);
}

void MultiViewImpl::SetViewBox(const vector &min, const vector &max)
{
	cam.pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.3f * ((float)NixScreenWidth / (float)NixTargetWidth);
	if (r > 0)
		cam.radius = r;
	update_zoom;
	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::ToggleWholeWindow()
{
	whole_window = !whole_window;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::ToggleGrid()
{
	grid_enabled = !grid_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::ToggleLight()
{
	light_enabled = !light_enabled;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::ToggleWire()
{
	wire_mode = !wire_mode;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::onCommand(const string & id)
{
	notifyBegin();
	//msg_write(id);

	if (id == "select_all")
		SelectAll();
	if (id == "select_none")
		SelectNone();
	if (id == "invert_selection")
		InvertSelection();

	if (id == "view_right")
		active_win->type = ViewRight;
	if (id == "view_left")
		active_win->type = ViewLeft;
	if (id == "view_front")
		active_win->type = ViewFront;
	if (id == "view_back")
		active_win->type = ViewBack;
	if (id == "view_top")
		active_win->type = ViewTop;
	if (id == "view_bottom")
		active_win->type = ViewBottom;
	if (id == "view_perspective")
		active_win->type = ViewPerspective;
	if (id == "view_isometric")
		active_win->type = ViewIsometric;
	if (id == "whole_window")
		ToggleWholeWindow();
	if (id == "grid")
		ToggleGrid();
	if (id == "light")
		ToggleLight();
	if (id == "wire")
		ToggleWire();

	if (id == "view_push")
		ViewStagePush();
	if (id == "view_pop")
		ViewStagePop();
	notifyEnd();
}

void MultiViewImpl::onMouseWheel()
{
	notifyBegin();
	HuiEvent *e = HuiGetEvent();

	// mouse wheel -> zoom
	if (e->dz > 0)
		CamZoom(SPEED_ZOOM_WHEEL, mouse_win->type != ViewPerspective);
	if (e->dz < 0)
		CamZoom(1.0f / SPEED_ZOOM_WHEEL, mouse_win->type != ViewPerspective);
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
		CamZoom(SPEED_ZOOM_KEY, mouse_win->type != ViewPerspective);
	if ((k == KEY_SUBTRACT) || (k == KEY_NUM_SUBTRACT))
		CamZoom(1.0f / SPEED_ZOOM_KEY, mouse_win->type != ViewPerspective);
	if (k == KEY_RIGHT)
		CamMove(-e_x * SPEED_MOVE);
	if (k == KEY_LEFT)
		CamMove( e_x * SPEED_MOVE);
	if (k == KEY_UP)
		CamMove( e_y * SPEED_MOVE);
	if (k == KEY_DOWN)
		CamMove(-e_y * SPEED_MOVE);
	if (k == KEY_SHIFT + KEY_UP)
		CamMove( e_z * SPEED_MOVE);
	if (k == KEY_SHIFT + KEY_DOWN)
		CamMove(-e_z * SPEED_MOVE);
	if (k == KEY_ESCAPE)
		action_con->endAction(false);
	notifyEnd();
}


int get_select_mode()
{
	if (NixGetKey(KEY_CONTROL))
		return MultiViewImpl::SelectAdd;
	if (NixGetKey(KEY_SHIFT))
		return MultiViewImpl::SelectInvert;
	return MultiViewImpl::SelectSet;
}

void MultiViewImpl::onLeftButtonDown()
{
	notifyBegin();
	UpdateMouse();

	// menu for selection of view type
	if ((menu) && (mouse_win->name_dest.inside(m.x, m.y))){
		active_win = mouse_win;
		menu->openPopup(ed, m.x, m.y);
		notifyEnd();
		return;
	}

	GetMouseOver();

	cam_con->onLeftButtonDown();


	if (cam_con->isMouseOver()){
		notifyEnd();
		return;
	}
	active_win = mouse_win;
	sel_rect.start_later(m);

	//v = v_0;
	if (allow_mouse_actions){
		if (action_con->isSelecting()){
			GetSelected(get_select_mode());
		}else if (action_con->leftButtonDown()){
		}
	}
	notifyEnd();
}



void MultiViewImpl::onMiddleButtonDown()
{
	notifyBegin();
	active_win = mouse_win;

// move camera?
	if (allow_infinite_scrolling)
		HoldCursor(true);
	ViewMoving = true;

	notify(MESSAGE_UPDATE);
	notifyEnd();
}



void MultiViewImpl::onRightButtonDown()
{
	notifyBegin();
	active_win = mouse_win;

// move camera?
	if (allow_infinite_scrolling)
		HoldCursor(true);
	ViewMoving = true;

	notify(MESSAGE_UPDATE);
	notifyEnd();
}



void MultiViewImpl::onMiddleButtonUp()
{
	notifyBegin();
	if (ViewMoving){
		ViewMoving = false;
		HoldCursor(false);
	}
	notifyEnd();
}



void MultiViewImpl::onRightButtonUp()
{
	notifyBegin();
	if (ViewMoving){
		ViewMoving = false;
		HoldCursor(false);
	}
	notifyEnd();
}



void MultiViewImpl::onKeyUp()
{
}



void MultiViewImpl::onLeftButtonUp()
{
	notifyBegin();
	EndRect();

	action_con->leftButtonUp();
	cam_con->onLeftButtonUp();
	notifyEnd();
}



void MultiViewImpl::UpdateMouse()
{
	m.x = HuiGetEvent()->mx;
	m.y = HuiGetEvent()->my;
	v.x = HuiGetEvent()->dx;
	v.y = HuiGetEvent()->dy;

	bool lbut = HuiGetEvent()->lbut;
	bool mbut = HuiGetEvent()->mbut;
	bool rbut = HuiGetEvent()->rbut;

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
	UpdateMouse();

	bool lbut = HuiGetEvent()->lbut;
	bool mbut = HuiGetEvent()->mbut;
	bool rbut = HuiGetEvent()->rbut;

	// hover
	if ((!action_con->inUse()) && (!cam_con->inUse()) && (!sel_rect.active))
		GetMouseOver();

	if ((lbut) && (action_con->isSelecting()) && (!cam_con->inUse()) && (allow_rect) && (!sel_rect.active)){
		sel_rect.dist += abs(v.x) + abs(v.y);
		if (sel_rect.dist >= MinMouseMoveToInteract)
			StartRect();
	}

	// rectangle
	if (sel_rect.active)
		SelectAllInRectangle(get_select_mode());

	// left button -> move data
	if (action_con->inUse())
		action_con->mouseMove();

	if (cam_con->inUse())
		cam_con->onMouseMove();


	if (ViewMoving){
		int t = active_win->type;
		if ((t == ViewPerspective) || (t == ViewIsometric)){
// camera rotation
			CamRotate(v, mbut || (NixGetKey(KEY_CONTROL)));
		}else{
// camera translation
			CamMove(v);
		}
	}

	// ignore mouse, while "holding"
	if (HoldingCursor){
		if (fabs(m.x - HoldingX) + fabs(m.y - HoldingY) > 100)
			ed->setCursorPos(HoldingX, HoldingY);
	}

	notify(MESSAGE_UPDATE);
	notifyEnd();
}




void MultiViewImpl::StartRect()
{
	sel_rect.active = true;

	// reset selection data
	foreach(DataSet &d, data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->m_old = sd->is_selected;
			}

	notify(MESSAGE_UPDATE);
}

void MultiViewImpl::EndRect()
{
	sel_rect.active = false;

	notify(MESSAGE_UPDATE);
}


#define GridConst	5.0f

float MultiViewImpl::GetGridD()
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


string MultiViewImpl::GetMVScaleByZoom(vector &v)
{

	float l = GetGridD() * 10.1f;
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

void MultiViewImpl::DrawMousePos()
{
	vector m = GetCursor3d();
	string unit = GetMVScaleByZoom(m);
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;

	if (mouse_win->type == View2D){
		ed->drawStr(MaxX, MaxY - 60, sx, Edward::AlignRight);
		ed->drawStr(MaxX, MaxY - 40, sy, Edward::AlignRight);
	}else{
		ed->drawStr(MaxX, MaxY - 80, sx, Edward::AlignRight);
		ed->drawStr(MaxX, MaxY - 60, sy, Edward::AlignRight);
		ed->drawStr(MaxX, MaxY - 40, sz, Edward::AlignRight);
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
		DrawMousePos();

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

void MultiViewImpl::SetMouseAction(const string & name, int mode)
{
	if ((!mode3d) && (mode == ActionRotate))
		mode = ActionRotate2d;
	action_con->action.name = name;
	action_con->action.mode = mode;
	action_con->disable();
	if (!action_con->isSelecting())
		action_con->enable();
}

void MultiViewImpl::SelectAll()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = true;
		}
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiViewImpl::SelectNone()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			sd->is_selected = false;
		}
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiViewImpl::InvertSelection()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = !sd->is_selected;
		}
	notify(MESSAGE_SELECTION_CHANGE);
}

bool MultiViewImpl::HasSelection()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->is_selected)
				return true;
		}
	return false;
}

vector MultiViewImpl::GetSelectionCenter()
{
	vector min, max;
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

vector MultiViewImpl::GetCursor3d()
{
	return mouse_win->unproject(m, cam.pos);
}

vector MultiViewImpl::GetCursor3d(const vector &depth_reference)
{
	return mouse_win->unproject(m, depth_reference);
}


void MultiViewImpl::GetMouseOver()
{
	msg_db_f("GetMouseOver",6);
	hover.reset();

	if (!ed->input.inside_smart)
		return;

	/*if (!MVSelectable)
		return;*/
	if (cam_con->isMouseOver())
		return;
	if (action_con->isMouseOver(hover.point))
		return;
	float _radius=(float)PointRadiusMouseOver;
	float z_min=1;
	foreachi(DataSet &d, data, di)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
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
					if (sd->is_selected)
						return;
				}
			}
}

void MultiViewImpl::UnselectAll()
{
	foreach(DataSet &d, data)
		if (d.selectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->is_selected = false;
			}
	notify(MESSAGE_SELECTION_CHANGE);
}

void MultiViewImpl::GetSelected(int mode)
{
	msg_db_f("GetSelected",4);
	notifyBegin();
	if ((hover.index < 0) || (hover.type < 0)){
		if (mode == SelectSet)
			UnselectAll();
	}else{
		SingleData* sd=MVGetSingleData(data[hover.set], hover.index);
		if (sd->is_selected){
			if (mode == SelectInvert){
				sd->is_selected=false;
			}
		}else{
			if (mode == SelectSet){
				UnselectAll();
				sd->is_selected=true;
			}else{
				sd->is_selected=true;
			}
		}
	}
	notify(MESSAGE_SELECTION_CHANGE);
	notifyEnd();
}

void MultiViewImpl::SelectAllInRectangle(int mode)
{
	msg_db_f("SelAllInRect",4);
	notifyBegin();
	// reset data
	UnselectAll();

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
				if (mode == SelectInvert)
					sd->is_selected = (sd->m_old && !sd->m_delta) || (!sd->m_old && sd->m_delta);
				else if (mode == SelectAdd)
					sd->is_selected = (sd->m_old || sd->m_delta);
				else
					sd->is_selected = sd->m_delta;
			}

	notify(MESSAGE_SELECTION_CHANGE);
	notifyEnd();
}

void MultiViewImpl::HoldCursor(bool holding)
{
	HoldingX = m.x;
	HoldingY = m.y;
	HoldingCursor = holding;
	ed->showCursor(!holding);
}



void MultiViewImpl::AddMessage3d(const string &str, const vector &pos)
{
	Message3d m;
	m.str = str;
	m.pos = pos;
	message3d.add(m);
}

void MultiViewImpl::SetAllowRect(bool allow)
{
	allow_rect = allow;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::SetAllowAction(bool allow)
{
	allow_mouse_actions = allow;
	notify(MESSAGE_SETTINGS_CHANGE);
}

void MultiViewImpl::ViewStagePush()
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

void MultiViewImpl::ViewStagePop()
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

void MultiViewImpl::ResetMessage3d()
{
	message3d.clear();
}

};
