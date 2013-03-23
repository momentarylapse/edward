/*
 * MultiView.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Edward.h"
#include "MultiView.h"


const color ColorBackGround3D = color(1,0,0,0.15f);
const color ColorBackGround2D = color(1,0,0,0.10f);
const color ColorGrid = color(1,0.7f,0.7f,0.7f);
const color ColorText = White;

const float SPEED_MOVE = 0.05f;
const float SPEED_ZOOM_KEY = 1.15f;
const float SPEED_ZOOM_WHEEL = 1.15f;

const int MinMouseMoveToInteract = 5;
const float MouseRotationSpeed = 0.0033f;

const int PointRadius = 2;
const int PointRadiusMouseOver = 4;

#define update_zoom		if (mode3d) \
							zoom = ((float)NixScreenHeight / (whole_window ? 1.0f : 2.0f) / radius); \
						else \
							zoom = (float)NixScreenHeight * 0.8f / radius;
#define MVGetSingleData(d, index)	((MultiViewSingleData*) ((char*)(d).data->data + (d).data->element_size* index))
//#define MVGetSingleData(d, index)	( dynamic_cast<MultiViewSingleData*> ((char*)(d).data + (d).DataSingleSize * index))

extern matrix NixProjectionMatrix;
extern matrix NixProjectionMatrix2d;

MultiViewSingleData::MultiViewSingleData()
{
	view_stage = 0;
	is_selected = false;
	m_delta = false;
	m_old = false;
	is_special = false;
	pos = v_0;
}

MultiView::MultiView(bool _mode3d) :
	Observable("MultiView")
{
	mode3d = _mode3d;

	if (mode3d){
		light = NixCreateLight();

		view[0].type = ViewBack;
		view[1].type = ViewLeft;
		view[2].type = ViewTop;
		view[3].type = ViewPerspective;
		view[4].type = ViewPerspective;

		// Menu
		menu = new CHuiMenu();
		menu->AddItem(_("Ansicht"), "view_menu_sign");
		menu->EnableItem("view_menu_sign", false);
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
		view[0].type = View2D;
		light = -1;
	}
	mx = my = 0;
	HoldingCursor = false;
	HoldingX = HoldingY = 0;

	Reset();
}

MultiView::~MultiView()
{
}

void MultiView::Reset()
{
	MVRectable = MVRect = false;
	mouse_win = 0;

	ViewMoving = -1;

	ResetData(NULL);
	ResetMouseAction();
	ResetView();
}

void MultiView::ResetView()
{
	pos = v_0;
	ang = v_0;
	if (mode3d)
		radius = 100;
	else
		radius = 1;
	zoom = 1;
	whole_window = false;
	grid_enabled = true;
	light_enabled = true;
	ignore_radius = false;
	wire_mode = false;

	view_stage = 0;

	MouseOver = -1;
	MouseOverSet = -1;
	MouseOverType = -1;
	Notify("SettingsChange");
}

void MultiView::ResetMouseAction()
{
	cur_action = NULL;
	action[0].reset();
	action[1].reset();
	action[2].reset();
	active_mouse_action = -1;
}

void MultiView::ResetData(Data *_data)
{
	data.clear();
	_data_ = _data;
	if (!_data_)
		ResetMouseAction();
}

void MultiView::SetData(int type, const DynamicArray & a, void *user_data, int mode, t_is_mouse_over_func *is_mouse_over_func, t_is_in_rect_func *is_in_rect_func)
{
	MultiViewData d;
	d.Type = type;
	d.data = (DynamicArray*)&a;
	d.user_data = user_data;
	d.MVSelectable = (mode & FlagSelect)>0;
	d.Drawable = (mode & FlagDraw)>0;
	d.Indexable = (mode & FlagIndex)>0;
	d.Movable = (mode & FlagMove)>0;
	d.IsMouseOver = is_mouse_over_func;
	d.IsInRect = is_in_rect_func;
	data.add(d);
}



void MultiView::DoZoom(float factor)
{
	vector mup;
	if (mode3d){
		if (view[mouse_win].type != ViewPerspective)
			mup = VecUnProject(vector(mx,my,0),mouse_win);
		radius /= factor;
		update_zoom;
		if (view[mouse_win].type != ViewPerspective)
			pos += mup - VecUnProject(vector(mx,my,0),mouse_win);
	}else{
		mup = VecUnProject(vector(mx,my,0),mouse_win);
		radius /= factor;
		update_zoom;
		pos += mup - VecUnProject(vector(mx,my,0),mouse_win);
	}
	Notify("Update");
}

void MultiView::DoMove(const vector &dir)
{
	vector d = GetDirection(mouse_win);
	vector u = GetDirectionUp(mouse_win);
	vector r = GetDirectionRight(mouse_win);
	if (mode3d)
		pos += radius*(r*dir.x+u*dir.y+d*dir.z) * SPEED_MOVE;
	else
		pos += (float)NixScreenHeight / zoom*(r*dir.x+u*dir.y) * SPEED_MOVE;
	Notify("Update");
}

void MultiView::SetViewBox(const vector &min, const vector &max)
{
	pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.3f * ((float)NixScreenWidth / (float)NixTargetWidth);
	if (r > 0)
		radius = r;
	update_zoom;
	Notify("Update");
}

void MultiView::ToggleWholeWindow()
{
	whole_window = !whole_window;
	view[4].type = view[mouse_win].type;
	Notify("SettingsChange");
}

void MultiView::ToggleGrid()
{
	grid_enabled = !grid_enabled;
	Notify("SettingsChange");
}

void MultiView::ToggleLight()
{
	light_enabled = !light_enabled;
	Notify("SettingsChange");
}

void MultiView::ToggleWire()
{
	wire_mode = !wire_mode;
	Notify("SettingsChange");
}

void MultiView::OnCommand(const string & id)
{
	HuiEvent *e = HuiGetEvent();

	//msg_write(id);

	if (id == "select_all")
		SelectAll();
	if (id == "select_none")
		SelectNone();
	if (id == "invert_selection")
		InvertSelection();

	if (id == "view_right")
		view[mouse_win].type = ViewRight;
	if (id == "view_left")
		view[mouse_win].type = ViewLeft;
	if (id == "view_front")
		view[mouse_win].type = ViewFront;
	if (id == "view_back")
		view[mouse_win].type = ViewBack;
	if (id == "view_top")
		view[mouse_win].type = ViewTop;
	if (id == "view_bottom")
		view[mouse_win].type = ViewBottom;
	if (id == "view_perspective")
		view[mouse_win].type = ViewPerspective;
	if (id == "view_isometric")
		view[mouse_win].type = ViewIsometric;
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

	// mouse wheel -> zoom
	if (e->dz > 0)
		DoZoom(SPEED_ZOOM_WHEEL);
	if (e->dz < 0)
		DoZoom(1.0f / SPEED_ZOOM_WHEEL);
}



void MultiView::OnKeyDown()
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
		MouseActionEnd(false);
}


int get_select_mode()
{
	if (NixGetKey(KEY_CONTROL))
		return MultiView::SelectAdd;
	if (NixGetKey(KEY_SHIFT))
		return MultiView::SelectInvert;
	return MultiView::SelectSet;
}

void MultiView::OnLeftButtonDown()
{
	// menu for selection of view type
	if (view[mouse_win].name_dest.inside(mx, my)){
		menu->OpenPopup(ed, mx, my);
		return;
	}

	MouseMovedSinceClick = 0;
	Moved = false;
	vx = vy = 0;
	GetSelected(get_select_mode());

	if (Selected<0){
		if (MVRectable)
			StartRect();
	}else{
	}
}



void MultiView::OnMiddleButtonDown()
{
	bool allow = true;
	if ((MouseOverType >= 0) && (MouseOver >= 0))
		if (MVGetSingleData(data[MouseOverSet], MouseOver)->is_selected)
			allow = false;
	if (allow){
// move camera?
		HoldCursor(true);
		ViewMoving = mouse_win;
		Selected = -1;
	}
}



void MultiView::OnRightButtonDown()
{
	bool allow = true;
	if ((MouseOverType >= 0) && (MouseOver >= 0))
		if (MVGetSingleData(data[MouseOverSet], MouseOver)->is_selected)
			allow = false;
	if (allow){
// move camera?
		HoldCursor(true);
		ViewMoving = mouse_win;
		Selected = -1;
	}else{
		MouseMovedSinceClick = 0;
		GetSelected();
	}
}



void MultiView::OnMiddleButtonUp()
{
	if (ViewMoving >= 0){
		ViewMoving = -1;
		HoldCursor(false);
	}
	MouseActionEnd(true);
}



void MultiView::OnRightButtonUp()
{
	if (ViewMoving >= 0){
		ViewMoving = -1;
		HoldCursor(false);
	}
	MouseActionEnd(true);
}



void MultiView::OnKeyUp()
{
}



void MultiView::OnLeftButtonUp()
{
	EndRect();
	MultiViewEditing = false;
	MouseActionEnd(true);
}



void MultiView::OnMouseMove()
{
	mx = HuiGetEvent()->mx;
	my = HuiGetEvent()->my;
	vx = HuiGetEvent()->dx;
	vy = HuiGetEvent()->dy;

	bool lbut = HuiGetEvent()->lbut;
	bool mbut = HuiGetEvent()->mbut;
	bool rbut = HuiGetEvent()->rbut;


	// which window is the cursor in?
	if (!cur_action){
		if ((mx<MaxX/2)&&(my<MaxY/2))
			mouse_win=0;
		if ((mx>MaxX/2)&&(my<MaxY/2))
			mouse_win=1;
		if ((mx<MaxX/2)&&(my>MaxY/2))
			mouse_win=2;
		if ((mx>MaxX/2)&&(my>MaxY/2))
			mouse_win=3;
		if (whole_window)
			mouse_win=4;
		if (!mode3d)
			mouse_win = 0;
	}


	// hover
	if ((!lbut) && (!mbut) && (!rbut))
		GetMouseOver();

	// rectangle
	if (MVRect)
		SelectAllInRectangle(get_select_mode());

	// left button -> move data
	//msg_write(lbut);
	if ((lbut) or (mbut) or (rbut)){
		int d = abs(vx) + abs(vy);
		MouseMovedSinceClick += d;
		if ((MouseMovedSinceClick >= MinMouseMoveToInteract) and (MouseMovedSinceClick - d < MinMouseMoveToInteract)){
			MultiViewEditing = true;
			if (Selected >= 0){
				MouseActionStart(rbut ? (NixGetKey(KEY_SHIFT) ? 1 : 2) : 0);
			}
		}
		MouseActionUpdate();
	}


	if (ViewMoving >= 0){
		int t = view[ViewMoving].type;
		if ((t == ViewPerspective) || (t == ViewIsometric)){
// camera rotation
			bool RotatingOwn = (mbut || (NixGetKey(KEY_CONTROL)));
			if (RotatingOwn)
				pos -= radius * ang.ang2dir();
			vector dang = vector((float)vy, (float)vx, 0) * MouseRotationSpeed;
			ang = VecAngAdd(dang, ang);
			if (RotatingOwn)
				pos += radius * ang.ang2dir();
		}else{
// camera translation
			if (t == View2D)
				pos += float(vx) / zoom * GetDirectionRight(ViewMoving) + float(vy) / zoom * GetDirectionUp(ViewMoving);
			else
				pos += float(vx) / zoom * GetDirectionRight(ViewMoving) + float(vy) / zoom * GetDirectionUp(ViewMoving);
		}
	}

	// ignore mouse, while "holding"
	if (HoldingCursor){
		if (fabs(mx - HoldingX) + fabs(my - HoldingY) > 100)
			ed->SetCursorPos(HoldingX, HoldingY);
	}

	Notify("Update");
}




void MultiView::StartRect()
{
	MVRect=true;
	RectWin=mouse_win;
	RectX=mx;
	RectY=my;

	// reset selection data
	foreach(MultiViewData &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				MultiViewSingleData* sd = MVGetSingleData(d,i);
				sd->m_old = sd->is_selected;
			}

	Notify("Update");
}

void MultiView::EndRect()
{
	/*if (PostEndRect)
		PostEndRect();*/

	MVRect=false;
	RectWin=-1;

	Notify("Update");
}


#define GridConst	5.0f

float MultiView::GetGridD()
{
	float z = zoom,d=1.0f;
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

float GetDensity(int i,float t)
{
	if (i%10==0)	t*=10;
	if (i%100==0)	t*=10;
	t=(float)sqrt(t)/60;
	if (t>0.6f)		t=0.6f;
	if (i==0)		t=1;
	return t;
}


void MultiView::DrawGrid(int win)
{
	int vt=view[win].type;
	if (vt == ViewIsometric)
		return;
	rect d;
	vector bg_a,bg_b;

	// Hintergrund-Bilder
	/*if(win<4)
	if (BgTexture[win]>=0){
		NixSetZ(false,false);
		bg_a=VecProject(BgTextureA[win],win);
		bg_b=VecProject(BgTextureB[win],win);
		d.x1=bg_a.x;
		d.x2=bg_b.x;
		d.y1=bg_a.y;
		d.y2=bg_b.y;
		NixDraw2D(BgTexture[win],White,r01,d,0.9999f);
		NixSetZ(true,true);
	}*/

// grid of coordinates
	if (!grid_enabled)
		return;

	if (vt == View2D){
		return;
	}

	// spherical for perspective view
	if (vt==ViewPerspective){
		vector PerspectiveViewPos = radius * ang.ang2dir() - pos;
		//NixSetZ(false,false);
		// horizontale
		float r=NixMaxDepth*0.6f;
		for (int j=-16;j<16;j++)
			for (int i=0;i<64;i++){
				vector pa = vector(float(j)/32*pi,float(i  )/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				vector pb = vector(float(j)/32*pi,float(i+1)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				NixSetColor(ColorInterpolate(ColorBackGround2D,ColorGrid,j==0?0.6f:0.1f));
				NixDrawLine3D(pa,pb);
			}
		// vertikale
		for (int j=0;j<32;j++)
			for (int i=0;i<64;i++){
				vector pa = vector(float(i  )/32*pi,float(j)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				vector pb = vector(float(i+1)/32*pi,float(j)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				NixSetColor(ColorInterpolate(ColorBackGround2D,ColorGrid,(j%16)==0?0.6f:0.1f));
				NixDrawLine3D(pa,pb);
			}
		//NixSetZ(true,true);
		return;
	}

	// rectangular
	float D = GetGridD();
	int a,b;
	float fa,fb,t;

	rect dest = GetRect(win);
	vector vux1 = VecUnProject(vector(dest.x1,0,0), win);
	vector vux2 = VecUnProject(vector(dest.x2,0,0), win);
	vector vuy1 = VecUnProject(vector(0,dest.y1,0), win);
	vector vuy2 = VecUnProject(vector(0,dest.y2,0), win);
	vector n,va,vb;

	// vertikal
	n=vux2-vux1;
	n/=n.length_fuzzy();	//n.normalize();
	va=n*VecDotProduct(n,vux1);
	vb=n*VecDotProduct(n,vux2);
	fa=(va.x+va.y+va.z)/D;
	fb=(vb.x+vb.y+vb.z)/D;
	if (fa>fb){	t=fa;	fa=fb;	fb=t;	}
	a=(int)fa;
	b=(int)fb+1;
	for (int i=a;i<b;i++){
		int x=(int)VecProject(vector((float)i*D,(float)i*D,(float)i*D),win).x;
		NixSetColor(ColorInterpolate(ColorBackGround2D,ColorGrid,GetDensity(i,(float)MaxX/(fb-fa))));
		NixDrawLineV(x,dest.y1,dest.y2,0.99998f-GetDensity(i,(float)MaxX/(fb-fa))*0.00005f);
	}

	// horizontal
	n=vuy2-vuy1;
	n/=n.length_fuzzy();	//-normalize(n);
	va=n*VecDotProduct(n,vuy1);
	vb=n*VecDotProduct(n,vuy2);
	fa=(va.x+va.y+va.z)/D;
	fb=(vb.x+vb.y+vb.z)/D;
	if (fa>fb){	t=fa;	fa=fb;	fb=t;	}
	a=(int)fa;
	b=(int)fb+1;
	for (int i=a;i<b;i++){
		int y=(int)VecProject(vector((float)i*D,(float)i*D,(float)i*D),win).y;
		NixSetColor(ColorInterpolate(ColorBackGround2D,ColorGrid,GetDensity(i,(float)MaxX/(fb-fa))));
		NixDrawLineH(dest.x1,dest.x2,y,0.99998f-GetDensity(i,(float)MaxX/(fb-fa))*0.00005f);
	}
}

string GetMVScaleByZoom(MultiView *mv, vector &v)
{

	float l = mv->GetGridD() * 10.1f;
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

void MultiView::DrawMousePos()
{
	vector m = GetCursor3d();
	string unit = GetMVScaleByZoom(this, m);
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;

	if (view[mouse_win].type == View2D){
		ed->DrawStr(MaxX, MaxY - 60, sx, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 40, sy, Edward::AlignRight);
	}else{
		ed->DrawStr(MaxX, MaxY - 80, sx, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 60, sy, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 40, sz, Edward::AlignRight);
	}
}


void MultiView::DrawWin(int win)
{
	msg_db_r("MultiView.DrawWin",2);
	matrix r, t;
	rect dest = GetRect(win);
	NixStartPart(dest.x1, dest.y1, dest.x2, dest.y2, true);
	string view_kind;
	MatrixIdentity(view[win].mat);
	NixEnableLighting(false);

	if (view[win].type == ViewPerspective){
		NixSetPerspectiveMode(PerspectiveCenterAutoTarget);
		if (whole_window)
			NixSetPerspectiveMode(PerspectiveSizeSet,(float)NixScreenWidth,(float)NixScreenHeight);
		else
			NixSetPerspectiveMode(PerspectiveSizeSet,(float)NixScreenWidth/2,(float)NixScreenHeight/2);
		NixSetPerspectiveMode(PerspectiveCenterSet, (dest.x1 + dest.x2) / 2, (dest.y1 + dest.y2) / 2);
		NixSetProjection(true, true);
		view[win].projection = NixProjectionMatrix;
		NixSetColor(ColorBackGround3D);
		NixDraw2D(r_id,NixTargetRect,0.9999999f);
	}else if (view[win].type == View2D){
		NixSetPerspectiveMode(Perspective2DScaleSet, zoom, zoom);
		NixSetColor(ColorBackGround2D);
		NixDraw2D(r_id,NixTargetRect,0.9999999f);
	}else{
		matrix s;
		MatrixTranslation(t, vector((dest.x2 + dest.x1) / 2, (dest.y2 + dest.y1) / 2, 0));
		MatrixScale(s, zoom, -zoom, zoom / 1000);
		view[win].projection = NixProjectionMatrix2d * t * s;
		NixSetProjectionMatrix(view[win].projection);
		NixSetColor(ColorBackGround2D);
		NixDraw2D(r_id,NixTargetRect,0.9999999f);
	}

	// camera position
	vector vt=-pos;
	MatrixTranslation(t,vt);
	if (view[win].type == ViewFront){
		view_kind = _("Vorne");
		view[win].ang = - e_y * pi;
	}else if (view[win].type == ViewBack){
		view_kind = _("Hinten");
		view[win].ang = v_0;
	}else if (view[win].type == ViewRight){
		view_kind = _("Rechts");
		view[win].ang = - e_y * pi / 2;
	}else if (view[win].type == ViewLeft){
		view_kind = _("Links");
		view[win].ang = e_y * pi / 2;
	}else if (view[win].type == ViewTop){
		view_kind = _("Oben");
		view[win].ang = e_x * pi / 2;
	}else if (view[win].type == ViewBottom){
		view_kind = _("Unten");
		view[win].ang = - e_x * pi / 2;
	}else if (view[win].type == ViewPerspective){
		view_kind = _("Perspektive");
		float _radius = ignore_radius ? 0 : radius;
		MatrixTranslation(t, _radius * ang.ang2dir() + vt);
		view[win].ang = ang;
	}else if (view[win].type == ViewIsometric){
		view_kind = _("Isometrisch");
		float _radius = ignore_radius? 0 : radius;
		MatrixTranslation(t, _radius * ang.ang2dir() + vt);
		view[win].ang = ang;
	}else if (view[win].type == View2D){
		view_kind = _("2D");
		vt=-pos;
		MatrixTranslation(t,vt);
		view[win].ang = - pi * e_y;
	}
	MatrixRotationView(r, view[win].ang);
	view[win].mat = r * t;
	cur_view = cur_view_rect = win;
	NixSetViewM(view[win].mat);
	//NixSetView(true,vector(0,0,-200),vector(0,0,0));
	NixSetZ(true,true);
	NixSetWire(false);
	NixEnableLighting(false);
	DrawGrid(win);

	NixSetWire(wire_mode);
	NixEnableLighting(light_enabled);
	NixSetMaterial(Black,White,Black,0,White);//Black);
	NixSetColor(White);

	// draw the actual data
	//msg_db_r("sub",2);
	if (ed->cur_mode)
		ed->cur_mode->OnDrawWinRecursive(win);
	//msg_db_l(2);

	// draw multiview data
	NixSetWire(false);
	NixEnableLighting(false);
	foreachi(MultiViewData &d, data, di){
		if ((d.Drawable)||(d.Indexable)){
			for (int i=0;i<d.data->num;i++){

				MultiViewSingleData *sd = MVGetSingleData(d, i);
				if (sd->view_stage < view_stage)
					continue;

				bool _di = ((d.Indexable) && (sd->is_selected) && (NixGetKey(KEY_I)));
				if ((!d.Drawable) && (!_di))
					continue;
				vector p = VecProject(sd->pos,win);
				if ((p.x<dest.x1)||(p.y<dest.y1)||(p.x>dest.x2)||(p.y>dest.y2)||(p.z<=0)||(p.z>=1))
					continue;
				if (_di)
					NixDrawStr(p.x+3, p.y, i2s(i));
				if (d.Drawable){
					color c=Blue;
					float radius=(float)PointRadius;
					float z=0.1f;
					if (sd->is_selected){
						c=Red;
						z=0.05f;
					}
					if (sd->is_special)
						c = Green;
					if ((MouseOverSet==di)&&(i==MouseOver)){
						c=color(c.a,c.r+0.4f,c.g+0.4f,c.b+0.4f);
						z=0.0f;
						radius=(float)PointRadiusMouseOver;
					}
					NixSetColor(c);
					NixDrawRect(	p.x-radius,
									p.x+radius,
									p.y-radius,
									p.y+radius,
									z);
				}
			}
		}
	}

	// DEBUG!
	//NixSetZ(false,false);
	//NixDrawLine3D(MouseOverTP,MouseOverTP+vector(0,ViewRadius/3,0),Green);

	// type of view

	view[win].name_dest = rect(dest.x1 + 3, dest.x1 + 3 + NixGetStrWidth(view_kind), dest.y1, dest.y1 + 20);
	NixSetColor(ColorText);
	if (view[win].name_dest.inside(mx, my))
		NixSetColor(Red);
	ed->DrawStr(dest.x1 + 3, dest.y1, view_kind);
	NixSetColor(ColorText);

	foreach(Message3d &m, message3d){
		vector p = VecProject(m.pos, win);
		if (p.z > 0)
			ed->DrawStr(p.x, p.y, m.str);
	}

	msg_db_l(2);
}

void MultiView::OnDraw()
{
	msg_db_r("Multiview.OnDraw",2);
	NixMaxDepth = radius * 1000;
	NixMinDepth = radius / 1000;

	update_zoom;

// light
	vector dir=-ang.ang2dir();
	color am=color(1,0.3f,0.3f,0.3f);
	color di=color(1,0.6f,0.6f,0.6f);
	color sp=color(1,0.4f,0.4f,0.4f);
	NixSetLightDirectional(light,dir,am,di,sp);
	NixEnableLight(light, true);
	NixEnableLighting(true);
	NixSetAmbientLight(Black);
	NixSetZ(true,true);
	NixSetColor(ColorText);


	view[0].dest = rect(0,MaxX/2,0,MaxY/2);
	view[1].dest = rect(MaxX/2,MaxX,0,MaxY/2);
	view[2].dest = rect(0,MaxX/2,MaxY/2,MaxY);
	view[3].dest = rect(MaxX/2,MaxX,MaxY/2,MaxY);
	view[4].dest = rect(0,MaxX,0,MaxY);

	if (!mode3d){
		view[0].dest = rect(0,MaxX,0,MaxY);
		DrawWin(0);
	}else if (whole_window){
		DrawWin(4);
	}else{
		// top left
		DrawWin(0);

		// top right
		DrawWin(1);

		// bottom left
		DrawWin(2);

		// bottom right
		DrawWin(3);

		NixStartPart(-1,-1,-1,-1,true);
		NixEnableLighting(false);
		NixSetColor(color(1,0.1f,0.1f,0.5f));
		NixDrawRect(0, MaxX, MaxY/2-1, MaxY/2+2, 0);
		NixDrawRect(MaxX/2-1, MaxX/2+2, 0, MaxY, 0);
	}
	cur_view = -1;
	cur_view_rect = -1;
	NixEnableLighting(false);

	if ((MVRectable)&&(MVRect)){
		NixSetZ(false, false);
		NixSetAlphaM(AlphaMaterial);
		NixSetColor(color(0.2f,0,0,1));
		NixDrawRect(mx,RectX,my,RectY,0);
		NixSetColor(color(0.7f,0,0,1));
		NixDrawLineV(RectX	,RectY	,my		,0);
		NixDrawLineV(mx		,RectY	,my		,0);
		NixDrawLineH(RectX	,mx		,RectY	,0);
		NixDrawLineH(RectX	,mx		,my		,0);
		NixSetAlphaM(AlphaNone);
		NixSetZ(true, true);
	}
	NixSetColor(ColorText);

	DrawMousePos();

	if (cur_action){
		int mode = action[active_mouse_action].mode;
		if (mode == ActionMove){
			vector t = mouse_action_param;
			string unit = GetMVScaleByZoom(this, t);
			ed->DrawStr(150, 100, f2s(t.x, 2) + " " + unit, Edward::AlignRight);
			ed->DrawStr(150, 120, f2s(t.y, 2) + " " + unit, Edward::AlignRight);
			ed->DrawStr(150, 140, f2s(t.z, 2) + " " + unit, Edward::AlignRight);
		}else if ((mode == ActionRotate) or (mode == ActionRotate2d)){
			vector r = mouse_action_param * 180.0f / pi;
			ed->DrawStr(150, 100, f2s(r.x, 2) + "°", Edward::AlignRight);
			ed->DrawStr(150, 120, f2s(r.y, 2) + "°", Edward::AlignRight);
			ed->DrawStr(150, 140, f2s(r.z, 2) + "°", Edward::AlignRight);
		}else if (mode == ActionScale){
			ed->DrawStr(150, 100, f2s(mouse_action_param.x * 100.0f, 2) + "%", Edward::AlignRight);
		}else if (mode == ActionScale2d){
			ed->DrawStr(150, 100, f2s(mouse_action_param.x * 100.0f, 2) + "%", Edward::AlignRight);
			ed->DrawStr(150, 120, f2s(mouse_action_param.y * 100.0f, 2) + "%", Edward::AlignRight);
		}
	}

	msg_db_l(2);
}

vector MultiView::VecUnProject2(const vector &p, const vector &o, int win)
{
	vector r;
	int t=view[win].type;
	vector pp = p;
	if ((t==ViewPerspective) || (t==ViewIsometric)){ // 3D
		if (cur_view!=win){
			cur_view=win;
			NixSetProjectionMatrix(view[win].projection);
			NixSetView(view[win].mat);
		}
		vector p_o;
		NixGetVecProject(p_o,o);
		pp.z=p_o.z;
		NixGetVecUnproject(r,pp);
	}else if (t==View2D){
		r.x=(p.x-MaxX/2)/zoom+pos.x;
		r.y=(p.y-MaxY/2)/zoom+pos.y;
		r.z=0;
	}else{ // 2D
		r=o;
		if (whole_window){
			pp.x-=MaxX/4;
			pp.y-=MaxY/4;
		}else{
			pp.x-=MaxX/2*float(win%2);
			pp.y-=MaxY/2*float(win/2);
		}
		if (t==ViewFront){
			r.x=-(pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewBack){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewRight){
			r.z= (pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewLeft){
			r.z=-(pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewTop){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z=-(pp.y-MaxY/4)/zoom+pos.z;
		}else if (t==ViewBottom){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z= (pp.y-MaxY/4)/zoom+pos.z;
		}
	}
	return r;
}

vector MultiView::VecProject(const vector &p,int win)
{
	vector r;
	int t=view[win].type;
	if ((t==ViewPerspective) || (t==ViewIsometric)){ // 3D
		if (cur_view!=win){
			cur_view=win;
			NixSetProjectionMatrix(view[win].projection);
			NixSetView(view[win].mat);
		}
		NixGetVecProject(r,p);
	}else if (t==View2D){
		r.x=MaxX/2+(p.x-pos.x)*zoom;
		r.y=MaxY/2+(p.y-pos.y)*zoom;
		r.z=0.5f;
	}else{ // 2D
		if (t==ViewFront){
			r.x=MaxX/4-(p.x-pos.x)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (t==ViewBack){
			r.x=MaxX/4+(p.x-pos.x)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (t==ViewRight){
			r.x=MaxX/4+(p.z-pos.z)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (t==ViewLeft){
			r.x=MaxX/4-(p.z-pos.z)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (t==ViewTop){
			r.x=MaxX/4+(p.x-pos.x)*zoom;
			r.y=MaxY/4-(p.z-pos.z)*zoom;
		}else if (t==ViewBottom){
			r.x=MaxX/4+(p.x-pos.x)*zoom;
			r.y=MaxY/4+(p.z-pos.z)*zoom;
		}
		if (whole_window){
			r.x+=MaxX/4;
			r.y+=MaxY/4;
		}else{
			r.x+=MaxX/2*float(win%2);
			r.y+=MaxY/2*float(win/2);
		}
		r.z=0.5f+VecDotProduct(p-pos,GetDirection(win))/radius/32;
	}
	return r;
}

vector MultiView::VecUnProject(const vector &p,int win)
{
	vector r;
	int t=view[win].type;
	vector pp = p;
	if ((t==ViewPerspective) || (t==ViewIsometric)){ // 3D
		if (cur_view!=win){
			cur_view=win;
			NixSetProjectionMatrix(view[win].projection);
			NixSetView(view[win].mat);
		}
		NixGetVecUnproject(r,pp);
	}else if (t==View2D){
		r.x=(pp.x-MaxX/2)/zoom+pos.x;
		r.y=(pp.y-MaxY/2)/zoom+pos.y;
		r.z=0;
	}else{ // 2D
		if (whole_window){
			pp.x-=MaxX/4;
			pp.y-=MaxY/4;
		}else{
			pp.x-=MaxX/2*float(win%2);
			pp.y-=MaxY/2*float(win/2);
		}
		r=pos;
		if (t==ViewFront){
			r.x=-(pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewBack){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewRight){
			r.z= (pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewLeft){
			r.z=-(pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewTop){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z=-(pp.y-MaxY/4)/zoom+pos.z;
		}else if (t==ViewBottom){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z= (pp.y-MaxY/4)/zoom+pos.z;
		}
	}
	return r;
}

vector MultiView::GetDirection(int win)
{
	int t=view[win].type;
	if ((t==ViewFront)||(t==View2D))
		return vector(0,0,-1);
	else if (t==ViewBack)
		return vector(0,0,1);
	else if (t==ViewRight)
		return vector(-1,0,0);
	else if (t==ViewLeft)
		return vector(1,0,0);
	else if (t==ViewTop)
		return vector(0,-1,0);
	else if (t==ViewBottom)
		return vector(0,1,0);
	else if ((t==ViewPerspective) || (t==ViewIsometric))
		return ang.ang2dir();
	return v_0;
}

vector MultiView::GetDirectionUp(int win)
{
	int t=view[win].type;
	if (t==View2D)
		return vector(0,-1,0);
	else if (t==ViewFront)
		return vector(0,1,0);
	else if (t==ViewBack)
		return vector(0,1,0);
	else if (t==ViewRight)
		return vector(0,1,0);
	else if (t==ViewLeft)
		return vector(0,1,0);
	else if (t==ViewTop)
		return vector(0,0,1);
	else if (t==ViewBottom)
		return vector(0,0,1);
	else if ((t==ViewPerspective) || (t==ViewIsometric))
		return VecAngAdd(vector(-pi/2,0,0), ang).ang2dir();
	return v_0;
}

vector MultiView::GetDirectionRight(int win)
{
	vector d=GetDirection(win);
	vector u=GetDirectionUp(win);
	return VecCrossProduct(d,u);
}

void MultiView::GetMovingFrame(vector &dir, vector &up, vector &right, int win)
{
	dir = GetDirection(win);
	up = GetDirectionUp(win);
	right = dir ^ up;
}

rect MultiView::GetRect(int win)
{
	return view[win].dest;
}

void MultiView::SetMouseAction(int button, const string & name, int mode)
{
	if ((!mode3d) && (mode == ActionRotate))
		mode = ActionRotate2d;
	action[button].name = name;
	action[button].mode = mode;
}

void MultiView::SelectAll()
{
	foreach(MultiViewData &d, data)
		for (int i=0;i<d.data->num;i++){
			MultiViewSingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = true;
		}
	Notify("SelectionChange");
}

void MultiView::SelectNone()
{
	foreach(MultiViewData &d, data)
		for (int i=0;i<d.data->num;i++){
			MultiViewSingleData* sd = MVGetSingleData(d, i);
			sd->is_selected = false;
		}
	Notify("SelectionChange");
}

void MultiView::InvertSelection()
{
	foreach(MultiViewData &d, data)
		for (int i=0;i<d.data->num;i++){
			MultiViewSingleData* sd = MVGetSingleData(d, i);
			if (sd->view_stage >= view_stage)
				sd->is_selected = !sd->is_selected;
		}
	Notify("SelectionChange");
}

vector MultiView::GetCursor3d()
{
	return VecUnProject2(vector(mx, my, 0), pos, mouse_win);
}

vector MultiView::GetCursor3d(const vector &depth_reference)
{
	return VecUnProject2(vector(mx, my, 0), depth_reference, mouse_win);
}


void MultiView::GetMouseOver()
{
	msg_db_r("GetMouseOver",6);
	MouseOver=MouseOverType=MouseOverSet=-1;
	/*if (!MVSelectable)
		return;*/
	float _radius=(float)PointRadiusMouseOver;
	float z_min=1;
	foreachi(MultiViewData &d, data, di)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				MultiViewSingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage < view_stage)
					continue;
				bool mo=false;
				vector mop;
				if (d.Drawable){
					vector p=VecProject(sd->pos, mouse_win);
					if ((p.z<=0)||(p.z>=1))
						continue;
					mo=((mx>=p.x-_radius)&&(mx<=p.x+_radius)&&(my>=p.y-_radius)&&(my<=p.y+_radius));
					if (mo){
						mop=sd->pos;
						z_min=0;
					}
				}
				if ((!mo)&&(d.IsMouseOver)){
					vector tp;
					mo=d.IsMouseOver(i, d.user_data, mouse_win, tp);
					if (mo){
						float z=VecProject(tp, mouse_win).z;
						if (z<z_min){
							z_min=z;
							mop=tp;
						}else{
							if (sd->is_selected){
								mop=tp;
							}else
								continue;
						}
					}
				}
				if (mo){
					MouseOver=i;
					MouseOverSet=di;
					MouseOverType=d.Type;
					MouseOverTP=mop;
					if (sd->is_selected){
						msg_db_l(6);
						return;
					}
				}
			}
	msg_db_l(6);
}

void MultiView::UnselectAll()
{
	foreach(MultiViewData &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				MultiViewSingleData* sd = MVGetSingleData(d,i);
				sd->is_selected = false;
			}
	MultiViewSelectionChanged = true;
	Notify("SelectionChange");
}

void MultiView::GetSelected(int mode)
{
	msg_db_r("GetSelected",4);
	NotifyBegin();
	Selected=MouseOver;
	SelectedType=MouseOverType;
	SelectedSet=MouseOverSet;
	SelectedTP=MouseOverTP;
	if ((Selected<0)||(SelectedType<0)){
		if (mode == SelectSet)
			UnselectAll();
	}else{
		MultiViewSingleData* sd=MVGetSingleData(data[SelectedSet],Selected);
		if (sd->is_selected){
			if (mode == SelectInvert){
				sd->is_selected=false;
				Selected=SelectedType=-1;
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
	MultiViewSelectionChanged=true;
	Notify("SelectionChange");
	NotifyEnd();
	msg_db_l(4);
}

void MultiView::SelectAllInRectangle(int mode)
{
	msg_db_r("SelAllInRect",4);
	NotifyBegin();
	// reset data
	UnselectAll();

	rect r = rect(min(mx, RectX), max(mx, RectX), min(my, RectY), max(my, RectY));

	// select
	foreach(MultiViewData &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				MultiViewSingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage<view_stage)
					continue;

				// selected?
				sd->m_delta=false;
				if (d.IsInRect){
					sd->m_delta = d.IsInRect(i, d.user_data, RectWin, &r);
				}else{// if (!sd->m_delta){
					vector p = VecProject(sd->pos,RectWin);
					sd->m_delta = r.inside(p.x, p.y);
				}

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

	msg_db_l(4);
}

void MultiView::HoldCursor(bool holding)
{
	HoldingX = mx;
	HoldingY = my;
	HoldingCursor = holding;
	ed->ShowCursor(!holding);
}

void MultiView::MouseActionStart(int button)
{
	if (cur_action)
		MouseActionEnd(false);
	if (action[button].name != ""){
		msg_write("mouse action start " + action[button].name);


		int mode = action[active_mouse_action].mode;
		mouse_action_param = v_0;
		if ((mode == ActionScale) or (mode == ActionScale2d))
			mouse_action_param = vector(1, 1, 1);

		active_mouse_action = button;
		mouse_action_pos0 = MouseOverTP;
		cur_action = ActionMultiViewFactory(action[button].name, _data_, mouse_action_param, mouse_action_pos0,
				GetDirectionRight(mouse_win), GetDirectionUp(mouse_win), GetDirection(mouse_win));
		cur_action->execute(_data_);
		_data_->Notify("Change");
		Notify("ActionStart");
	}
}


vector transform_ang(MultiView *mv, const vector &ang)
{
	quaternion qmv, mqmv, qang, q;
	QuaternionRotationV(qmv,  mv->view[mv->mouse_win].ang);
	QuaternionRotationV(qang, ang);
	mqmv = qmv;
	mqmv.inverse();
	q = qang * mqmv;
	q = qmv * q;
	return q.get_angles();
}

void MultiView::MouseActionUpdate()
{
	if (cur_action){
		//msg_write("mouse action update");

		vector v2p = vector(mx, my, 0);
		vector v2  = VecUnProject2(v2p, mouse_action_pos0, mouse_win);
		vector v1  = mouse_action_pos0;
		vector v1p = VecProject(v1, mouse_win);
		int mode = action[active_mouse_action].mode;
		if (mode == ActionMove)
			mouse_action_param = v2 - v1;
		else if (mode == ActionRotate)
			mouse_action_param = transform_ang(this, vector(v1p.y - v2p.y, v1p.x - v2p.x, 0) * 0.003f);
		else if (mode == ActionRotate2d)
			mouse_action_param = transform_ang(this, e_z * (v2p.x - v1p.x) * 0.003f);
		else if (mode == ActionScale)
			mouse_action_param = vector(1, 1, 1) * ( 1 + (v2p.x - v1p.x) * 0.01f);
		else if (mode == ActionScale2d)
			mouse_action_param = vector(1 + (v2p.x - v1p.x) * 0.01f, 1 - (v2p.y - v1p.y) * 0.01f, 1);
		else if (mode == ActionOnce)
			mouse_action_param = GetDirectionRight(mouse_win);
		else
			mouse_action_param = v_0;
		cur_action->undo(_data_);
		delete(cur_action);
		cur_action = ActionMultiViewFactory(action[active_mouse_action].name, _data_, mouse_action_param, mouse_action_pos0,
				GetDirectionRight(mouse_win), GetDirectionUp(mouse_win), GetDirection(mouse_win));
		cur_action->execute(_data_);
		_data_->Notify("Changed");

		Notify("ActionUpdate");
	}
}



void MultiView::MouseActionEnd(bool set)
{
	if (cur_action){
		msg_write("mouse action end");
		if (set){
			cur_action->undo(_data_);
					delete(cur_action);
			cur_action = ActionMultiViewFactory(action[active_mouse_action].name, _data_, mouse_action_param, mouse_action_pos0,
					GetDirectionRight(mouse_win), GetDirectionUp(mouse_win), GetDirection(mouse_win));
			_data_->Execute(cur_action);
			Notify("ActionExecute");
		}else{
			cur_action->abort_and_notify(_data_);
			delete(cur_action);
			Notify("ActionAbort");
		}
	}
	cur_action = NULL;
	active_mouse_action = -1;
}



void MultiView::AddMessage3d(const string &str, const vector &pos)
{
	Message3d m;
	m.str = str;
	m.pos = pos;
	message3d.add(m);
}

void MultiView::ViewStagePush()
{
	view_stage ++;

	foreach(MultiViewData &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				MultiViewSingleData* sd=MVGetSingleData(d,i);
				if (sd->is_selected)
					sd->view_stage = view_stage;
			}
	Notify("SettingsChange");
}

void MultiView::ViewStagePop()
{
	if (view_stage <= 0)
		return;
	view_stage --;
	foreach(MultiViewData &d, data)
		if (d.MVSelectable)
			for (int i=0;i<d.data->num;i++){
				MultiViewSingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage > view_stage)
					sd->view_stage = view_stage;
			}
	Notify("SettingsChange");
}

void MultiView::ResetMessage3d()
{
	message3d.clear();
}
