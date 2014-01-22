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
#include "SingleData.h"

namespace MultiView{

const float SPEED_MOVE = 0.05f;
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
	ColorBackGround3D = color(1,0,0,0.15f);
	ColorBackGround2D = color(1,0,0,0.10f);
	ColorGrid = color(1,0.7f,0.7f,0.7f);
	ColorText = White;
	ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	ColorPoint = color(1, 0.2f, 0.2f, 0.9f);
	ColorPointSelected = color(1, 0.9f, 0.2f, 0.2f);
	ColorPointSpecial = color(1, 0.2f, 0.8f, 0.2f);
	ColorWindowSeparator = color(1, 0.1f, 0.1f, 0.6f); // color(1,0.1f,0.1f,0.5f)
	ColorSelectionRect = color(0.2f,0,0,1);
	ColorSelectionRectBoundary = color(0.7f,0,0,1);

	PointRadius = 2;
	PointRadiusMouseOver = 4;

	mode3d = _mode3d;
	win[0] = new Window(this, ViewBack);
	win[1] = new Window(this, ViewLeft);
	win[2] = new Window(this, ViewTop);
	win[3] = new Window(this, ViewPerspective);

	if (mode3d){
		light = 0;

		// Menu
		menu = new HuiMenu;
		menu->AddItem(_("Ansicht"), "view_menu_sign");
		//menu->EnableItem("view_menu_sign", false);
		menu->AddSeparator();
		menu->AddItem(_("Rechts (-x)"), "view_right");
		menu->AddItem(_("Links (x)"), "view_left");
		menu->AddItem(_("Vorne (z)"), "view_front");
		menu->AddItem(_("Hinten (-z)"), "view_back");
		menu->AddItem(_("Oben (-y)"), "view_top");
		menu->AddItem(_("Unten (y)"), "view_bottom");
		menu->AddItem(_("Isometrisch"), "view_isometric");
		menu->AddItem(_("Perspektive"), "view_perspective");
	}else{
		win[0]->type = View2D;
		light = -1;
	}
	action_con = new ActionController(this);
	m = v_0;
	HoldingCursor = false;
	HoldingX = HoldingY = 0;
	allow_mouse_actions = true;

	Reset();
}

MultiViewImpl::~MultiViewImpl()
{
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

	selection.reset();
	action_con->reset();

	ResetData(NULL);
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
	Notify("SettingsChange");
}

void MultiViewImpl::ResetMouseAction()
{
	action_con->cur_action = NULL;
	action_con->action.reset();
}

void MultiViewImpl::ResetData(Data *_data)
{
	data.clear();
	action_con->data = _data;
	if (!_data)
		ResetMouseAction();
}

void MultiViewImpl::SetData(int type, const DynamicArray & a, void *user_data, int mode)
{
	DataSet d;
	d.Type = type;
	d.data = (DynamicArray*)&a;
	d.user_data = user_data;
	d.MVSelectable = (mode & FlagSelect)>0;
	d.Drawable = (mode & FlagDraw)>0;
	d.Indexable = (mode & FlagIndex)>0;
	d.Movable = (mode & FlagMove)>0;
	data.add(d);
}

void MultiViewImpl::SetViewStage(int *view_stage, bool allow_handle)
{}



void MultiViewImpl::DoZoom(float factor)
{
	vector mup;
	if (mode3d){
		if (mouse_win->type != ViewPerspective)
			mup = mouse_win->Unproject(m);
		cam.radius /= factor;
		update_zoom;
		if (mouse_win->type != ViewPerspective)
			cam.pos += mup - mouse_win->Unproject(m);
	}else{
		mup = mouse_win->Unproject(m);
		cam.radius /= factor;
		update_zoom;
		cam.pos += mup - mouse_win->Unproject(m);
	}
	if (action_con->show)
		action_con->Update();
	Notify("Update");
}

void MultiViewImpl::DoMove(const vector &dir)
{
	vector d, u, r;
	mouse_win->GetMovingFrame(d, u, r);
	if (mode3d)
		cam.pos += cam.radius*(r*dir.x+u*dir.y+d*dir.z) * SPEED_MOVE;
	else
		cam.pos += (float)NixScreenHeight / cam.zoom*(r*dir.x+u*dir.y) * SPEED_MOVE;
	Notify("Update");
}

void MultiViewImpl::SetViewBox(const vector &min, const vector &max)
{
	cam.pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.3f * ((float)NixScreenWidth / (float)NixTargetWidth);
	if (r > 0)
		cam.radius = r;
	update_zoom;
	Notify("Update");
}

void MultiViewImpl::ToggleWholeWindow()
{
	whole_window = !whole_window;
	Notify("SettingsChange");
}

void MultiViewImpl::ToggleGrid()
{
	grid_enabled = !grid_enabled;
	Notify("SettingsChange");
}

void MultiViewImpl::ToggleLight()
{
	light_enabled = !light_enabled;
	Notify("SettingsChange");
}

void MultiViewImpl::ToggleWire()
{
	wire_mode = !wire_mode;
	Notify("SettingsChange");
}

void MultiViewImpl::OnCommand(const string & id)
{
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
}

void MultiViewImpl::OnMouseWheel()
{
	HuiEvent *e = HuiGetEvent();

	// mouse wheel -> zoom
	if (e->dz > 0)
		DoZoom(SPEED_ZOOM_WHEEL);
	if (e->dz < 0)
		DoZoom(1.0f / SPEED_ZOOM_WHEEL);
}



void MultiViewImpl::OnKeyDown()
{
	int k = HuiGetEvent()->key_code;

	if ((k == KEY_ADD) ||(k == KEY_NUM_ADD))
		DoZoom(SPEED_ZOOM_KEY);
	if ((k == KEY_SUBTRACT) || (k == KEY_NUM_SUBTRACT))
		DoZoom(1.0f / SPEED_ZOOM_KEY);
	if (k == KEY_RIGHT)
		DoMove(-e_x);
	if (k == KEY_LEFT)
		DoMove( e_x);
	if (k == KEY_UP)
		DoMove( e_y);
	if (k == KEY_DOWN)
		DoMove(-e_y);
	if (k == KEY_SHIFT + KEY_UP)
		DoMove( e_z);
	if (k == KEY_SHIFT + KEY_DOWN)
		DoMove(-e_z);
	if (k == KEY_ESCAPE)
		action_con->EndAction(false);
}


int get_select_mode()
{
	if (NixGetKey(KEY_CONTROL))
		return MultiViewImpl::SelectAdd;
	if (NixGetKey(KEY_SHIFT))
		return MultiViewImpl::SelectInvert;
	return MultiViewImpl::SelectSet;
}

void MultiViewImpl::OnLeftButtonDown()
{
	UpdateMouse();
	active_win = mouse_win;

	// menu for selection of view type
	if ((menu) && (active_win->name_dest.inside(m.x, m.y))){
		menu->OpenPopup(ed, m.x, m.y);
		return;
	}

	GetMouseOver();

	sel_rect.start_later(m);
	//v = v_0;
	if (allow_mouse_actions){
		if (action_con->action.mode == ActionSelect){
			GetSelected(get_select_mode());

		}else if (action_con->LeftButtonDown()){
		}
	}
}



void MultiViewImpl::OnMiddleButtonDown()
{
	active_win = mouse_win;
	/*bool allow = true;
	if ((MouseOverType >= 0) && (MouseOver >= 0))
		if (MVGetSingleData(data[MouseOverSet], MouseOver)->is_selected)
			allow = false;
	if (allow){*/
// move camera?
		HoldCursor(true);
		ViewMoving = true;
		selection.reset();
	//}
	Notify("Update");
}



void MultiViewImpl::OnRightButtonDown()
{
	active_win = mouse_win;
	/*bool allow = true;
	if ((MouseOverType >= 0) && (MouseOver >= 0))
		if (MVGetSingleData(data[MouseOverSet], MouseOver)->is_selected)
			allow = false;
	if (allow){*/
// move camera?
		HoldCursor(true);
		ViewMoving = true;
		selection.reset();
	/*}else{
		MouseMovedSinceClick = 0;
		GetSelected();
	}*/
	Notify("Update");
}



void MultiViewImpl::OnMiddleButtonUp()
{
	if (ViewMoving){
		ViewMoving = false;
		HoldCursor(false);
	}
}



void MultiViewImpl::OnRightButtonUp()
{
	if (ViewMoving){
		ViewMoving = false;
		HoldCursor(false);
	}
}



void MultiViewImpl::OnKeyUp()
{
}



void MultiViewImpl::OnLeftButtonUp()
{
	EndRect();

	action_con->LeftButtonUp();
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


void MultiViewImpl::OnMouseMove()
{
	UpdateMouse();

	bool lbut = HuiGetEvent()->lbut;
	bool mbut = HuiGetEvent()->mbut;
	bool rbut = HuiGetEvent()->rbut;

	// hover
	if ((!action_con->InUse()) && (!sel_rect.active))
		GetMouseOver();

	if ((lbut) && (action_con->action.mode == ActionSelect) && (allow_rect) && (!sel_rect.active)){
		sel_rect.dist += abs(v.x) + abs(v.y);
		if (sel_rect.dist >= MinMouseMoveToInteract)
			StartRect();
	}

	// rectangle
	if (sel_rect.active)
		SelectAllInRectangle(get_select_mode());

	// left button -> move data
	if (action_con->InUse())
		action_con->MouseMove();


	if (ViewMoving){
		int t = active_win->type;
		if ((t == ViewPerspective) || (t == ViewIsometric)){
// camera rotation
			bool RotatingOwn = (mbut || (NixGetKey(KEY_CONTROL)));
			if (RotatingOwn)
				cam.pos -= cam.radius * (cam.ang * e_z);
			quaternion dang;
			QuaternionRotationV(dang, vector(v.y, v.x, 0) * MouseRotationSpeed);
			cam.ang = cam.ang * dang;
			if (RotatingOwn)
				cam.pos += cam.radius * (cam.ang * e_z);
		}else{
// camera translation
			vector r = active_win->GetDirectionRight();
			vector u = active_win->GetDirectionUp();
			cam.pos += float(v.x) / cam.zoom * r + float(v.y) / cam.zoom * u;
		}
	}

	// ignore mouse, while "holding"
	if (HoldingCursor){
		if (fabs(m.x - HoldingX) + fabs(m.y - HoldingY) > 100)
			ed->SetCursorPos(HoldingX, HoldingY);
	}

	Notify("Update");
}




void MultiViewImpl::StartRect()
{
	sel_rect.active = true;

	// reset selection data
	foreach(DataSet &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->m_old = sd->is_selected;
			}

	Notify("Update");
}

void MultiViewImpl::EndRect()
{
	sel_rect.active = false;

	Notify("Update");
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
		ed->DrawStr(MaxX, MaxY - 60, sx, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 40, sy, Edward::AlignRight);
	}else{
		ed->DrawStr(MaxX, MaxY - 80, sx, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 60, sy, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 40, sz, Edward::AlignRight);
	}
}


void MultiViewImpl::OnDraw()
{
	msg_db_f("Multiview.OnDraw",2);

	update_zoom;

	NixSetZ(true,true);
	NixSetColor(ColorText);



	if (!mode3d){
		win[0]->dest = rect(0,MaxX,0,MaxY);
		win[0]->Draw();
	}else if (whole_window){
		active_win->dest = rect(0,MaxX,0,MaxY);
		active_win->Draw();
	}else{
		// top left
		win[0]->dest = rect(0,MaxX/2,0,MaxY/2);
		win[0]->Draw();

		// top right
		win[1]->dest = rect(MaxX/2,MaxX,0,MaxY/2);
		win[1]->Draw();

		// bottom left
		win[2]->dest = rect(0,MaxX/2,MaxY/2,MaxY);
		win[2]->Draw();

		// bottom right
		win[3]->dest = rect(MaxX/2,MaxX,MaxY/2,MaxY);
		win[3]->Draw();

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

	NixSetColor(ColorText);

	DrawMousePos();

	if (action_con->InUse())
		action_con->DrawParams();
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
	action_con->Disable();
	if (action_con->action.mode > ActionSelect)
		action_con->Enable();
}

void MultiViewImpl::SelectAll()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = true;
		}
	Notify("SelectionChange");
}

void MultiViewImpl::SelectNone()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			sd->is_selected = false;
		}
	Notify("SelectionChange");
}

void MultiViewImpl::InvertSelection()
{
	foreach(DataSet &d, data)
		for (int i=0;i<d.data->num;i++){
			SingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = !sd->is_selected;
		}
	Notify("SelectionChange");
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
	return mouse_win->Unproject(m, cam.pos);
}

vector MultiViewImpl::GetCursor3d(const vector &depth_reference)
{
	return mouse_win->Unproject(m, depth_reference);
}


void MultiViewImpl::GetMouseOver()
{
	msg_db_f("GetMouseOver",6);
	hover.reset();
	/*if (!MVSelectable)
		return;*/
	if (action_con->IsMouseOver(hover.point))
		return;
	float _radius=(float)PointRadiusMouseOver;
	float z_min=1;
	foreachi(DataSet &d, data, di)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage < view_stage)
					continue;
				float z;
				vector tp, mop;
				bool mo = sd->Hover(mouse_win, m, tp, z, d.user_data);
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
					hover.type = d.Type;
					hover.point = mop;
					if (sd->is_selected)
						return;
				}
			}
}

void MultiViewImpl::UnselectAll()
{
	foreach(DataSet &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd = MVGetSingleData(d,i);
				sd->is_selected = false;
			}
	Notify("SelectionChange");
}

void MultiViewImpl::GetSelected(int mode)
{
	msg_db_f("GetSelected",4);
	NotifyBegin();
	selection = hover;
	if ((selection.index < 0) || (selection.type < 0)){
		if (mode == SelectSet)
			UnselectAll();
	}else{
		SingleData* sd=MVGetSingleData(data[selection.set], selection.index);
		if (sd->is_selected){
			if (mode == SelectInvert){
				sd->is_selected=false;
				selection.index = selection.type = -1;
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
	Notify("SelectionChange");
	NotifyEnd();
}

void MultiViewImpl::SelectAllInRectangle(int mode)
{
	msg_db_f("SelAllInRect",4);
	NotifyBegin();
	// reset data
	UnselectAll();

	rect r = sel_rect.get(m);

	// select
	foreach(DataSet &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage<view_stage)
					continue;

				// selected?
				sd->m_delta = sd->InRect(active_win, r, d.user_data);

				// add the selection layers
				if (mode == SelectInvert)
					sd->is_selected = (sd->m_old && !sd->m_delta) || (!sd->m_old && sd->m_delta);
				else if (mode == SelectAdd)
					sd->is_selected = (sd->m_old || sd->m_delta);
				else
					sd->is_selected = sd->m_delta;
			}

	Notify("SelectionChange");
	NotifyEnd();
}

void MultiViewImpl::HoldCursor(bool holding)
{
	HoldingX = m.x;
	HoldingY = m.y;
	HoldingCursor = holding;
	ed->ShowCursor(!holding);
}



void MultiViewImpl::AddMessage3d(const string &str, const vector &pos)
{
	Message3d m;
	m.str = str;
	m.pos = pos;
	message3d.add(m);
}

void MultiViewImpl::ViewStagePush()
{
	view_stage ++;

	foreach(DataSet &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->is_selected)
					sd->view_stage = view_stage;
			}
	Notify("SettingsChange");
}

void MultiViewImpl::ViewStagePop()
{
	if (view_stage <= 0)
		return;
	view_stage --;
	foreach(DataSet &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				SingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage > view_stage)
					sd->view_stage = view_stage;
			}
	Notify("SettingsChange");
}

void MultiViewImpl::ResetMessage3d()
{
	message3d.clear();
}

};
