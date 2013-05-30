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
const color ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);

const float SPEED_MOVE = 0.05f;
const float SPEED_ZOOM_KEY = 1.15f;
const float SPEED_ZOOM_WHEEL = 1.15f;

const int MinMouseMoveToInteract = 5;
const float MouseRotationSpeed = 0.0033f;

const int PointRadius = 2;
const int PointRadiusMouseOver = 4;

#define update_zoom		\
	if (mode3d) \
		cam.zoom = ((float)NixScreenHeight / (whole_window ? 1.0f : 2.0f) / cam.radius); \
	else \
		cam.zoom = (float)NixScreenHeight * 0.8f / cam.radius;
#define MVGetSingleData(d, index)	((MultiViewSingleData*) ((char*)(d).data->data + (d).data->element_size* index))
//#define MVGetSingleData(d, index)	( dynamic_cast<MultiViewSingleData*> ((char*)(d).data + (d).DataSingleSize * index))

extern matrix NixViewMatrix;
extern matrix NixProjectionMatrix;

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
		light = 0;

		win[0].type = ViewBack;
		win[1].type = ViewLeft;
		win[2].type = ViewTop;
		win[3].type = ViewPerspective;

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
		win[0].type = View2D;
		light = -1;
	}
	for (int i=0;i<4;i++){
		win[i].multi_view = this;
		win[i].cam = &cam;
	}
	m = v_0;
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
	mouse_win = &win[0];
	active_win = &win[0];

	ViewMoving = false;

	ResetData(NULL);
	ResetMouseAction();
	ResetView();
}

void MultiView::ResetView()
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
	Notify("Update");
}

void MultiView::DoMove(const vector &dir)
{
	vector d, u, r;
	mouse_win->GetMovingFrame(d, u, r);
	if (mode3d)
		cam.pos += cam.radius*(r*dir.x+u*dir.y+d*dir.z) * SPEED_MOVE;
	else
		cam.pos += (float)NixScreenHeight / cam.zoom*(r*dir.x+u*dir.y) * SPEED_MOVE;
	Notify("Update");
}

void MultiView::SetViewBox(const vector &min, const vector &max)
{
	cam.pos = (min + max) / 2;
	float r = (max - min).length_fuzzy() * 1.3f * ((float)NixScreenWidth / (float)NixTargetWidth);
	if (r > 0)
		cam.radius = r;
	update_zoom;
	Notify("Update");
}

void MultiView::ToggleWholeWindow()
{
	whole_window = !whole_window;
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
	active_win = mouse_win;
	// menu for selection of view type
	if ((menu) && (active_win->name_dest.inside(m.x, m.y))){
		menu->OpenPopup(ed, m.x, m.y);
		return;
	}

	MouseMovedSinceClick = 0;
	Moved = false;
	v = v_0;
	GetSelected(get_select_mode());

	if (Selected<0){
		if (MVRectable)
			StartRect();
	}else{
	}
}



void MultiView::OnMiddleButtonDown()
{
	active_win = mouse_win;
	bool allow = true;
	if ((MouseOverType >= 0) && (MouseOver >= 0))
		if (MVGetSingleData(data[MouseOverSet], MouseOver)->is_selected)
			allow = false;
	if (allow){
// move camera?
		HoldCursor(true);
		ViewMoving = true;
		Selected = -1;
	}
	Notify("Update");
}



void MultiView::OnRightButtonDown()
{
	active_win = mouse_win;
	bool allow = true;
	if ((MouseOverType >= 0) && (MouseOver >= 0))
		if (MVGetSingleData(data[MouseOverSet], MouseOver)->is_selected)
			allow = false;
	if (allow){
// move camera?
		HoldCursor(true);
		ViewMoving = true;
		Selected = -1;
	}else{
		MouseMovedSinceClick = 0;
		GetSelected();
	}
	Notify("Update");
}



void MultiView::OnMiddleButtonUp()
{
	if (ViewMoving){
		ViewMoving = false;
		HoldCursor(false);
	}
	MouseActionEnd(true);
}



void MultiView::OnRightButtonUp()
{
	if (ViewMoving){
		ViewMoving = false;
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
				mouse_win = &win[0];
			if ((m.x>MaxX/2)&&(m.y<MaxY/2))
				mouse_win = &win[1];
			if ((m.x<MaxX/2)&&(m.y>MaxY/2))
				mouse_win = &win[2];
			if ((m.x>MaxX/2)&&(m.y>MaxY/2))
				mouse_win = &win[3];
		}
	}else{
		mouse_win = &win[0];
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
		int d = abs(v.x) + abs(v.y);
		MouseMovedSinceClick += d;
		if ((MouseMovedSinceClick >= MinMouseMoveToInteract) and (MouseMovedSinceClick - d < MinMouseMoveToInteract)){
			MultiViewEditing = true;
			if (Selected >= 0){
				MouseActionStart(rbut ? (NixGetKey(KEY_SHIFT) ? 1 : 2) : 0);
			}
		}
		MouseActionUpdate();
	}


	if (ViewMoving){
		int t = active_win->type;
		if ((t == ViewPerspective) || (t == ViewIsometric)){
// camera rotation
			bool RotatingOwn = (mbut || (NixGetKey(KEY_CONTROL)));
			if (RotatingOwn)
				cam.pos -= cam.radius * cam.ang.ang2dir();
			vector dang = vector(v.y, v.x, 0) * MouseRotationSpeed;
			cam.ang = VecAngAdd(dang, cam.ang);
			if (RotatingOwn)
				cam.pos += cam.radius * cam.ang.ang2dir();
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




void MultiView::StartRect()
{
	MVRect = true;
	RectX = m.x;
	RectY = m.y;

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

	MVRect = false;

	Notify("Update");
}


#define GridConst	5.0f

float MultiView::GetGridD()
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

float GetDensity(int i,float t)
{
	if (i%10==0)	t*=10;
	if (i%100==0)	t*=10;
	t=(float)sqrt(t)/60;
	if (t>0.6f)		t=0.6f;
	if (i==0)		t=1;
	return t;
}


void MultiViewWindow::DrawGrid()
{
	if (type == ViewIsometric)
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

	if (type == View2D)
		return;

	// spherical for perspective view
	if (type == ViewPerspective){
		vector PerspectiveViewPos = cam->radius * cam->ang.ang2dir() - cam->pos;
		//NixSetZ(false,false);
		// horizontal
		float r = cam->radius * 1000 * 0.6f;
		for (int j=-16;j<16;j++)
			for (int i=0;i<64;i++){
				vector pa = vector(float(j)/32*pi,float(i  )/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				vector pb = vector(float(j)/32*pi,float(i+1)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				NixSetColor(ColorInterpolate(ColorBackGround2D,ColorGrid,j==0?0.6f:0.1f));
				NixDrawLine3D(pa,pb);
			}
		// vertical
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
	float D = multi_view->GetGridD();
	int a,b;
	float fa,fb,t;

	vector vux1 = Unproject(vector(dest.x1,0,0));
	vector vux2 = Unproject(vector(dest.x2,0,0));
	vector vuy1 = Unproject(vector(0,dest.y1,0));
	vector vuy2 = Unproject(vector(0,dest.y2,0));
	vector n,va,vb;

	// vertical
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
		int x=(int)Project(vector((float)i*D,(float)i*D,(float)i*D)).x;
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
		int y=(int)Project(vector((float)i*D,(float)i*D,(float)i*D)).y;
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

	if (mouse_win->type == View2D){
		ed->DrawStr(MaxX, MaxY - 60, sx, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 40, sy, Edward::AlignRight);
	}else{
		ed->DrawStr(MaxX, MaxY - 80, sx, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 60, sy, Edward::AlignRight);
		ed->DrawStr(MaxX, MaxY - 40, sz, Edward::AlignRight);
	}
}


void MultiViewWindow::Draw()
{
	msg_db_r("MultiView.DrawWin",2);
	matrix rot, trans;
	NixScissor(dest);
	string view_kind;
	MatrixIdentity(mat);
	NixEnableLighting(false);
	NixSetTexture(-1);

	color bg = ColorBackGround2D;
	float height = NixScreenHeight;
	if (!multi_view->whole_window)
		height /= 2;

	// projection matrix
	if (type == ViewPerspective){
		NixSetProjectionPerspectiveExt((dest.x1 + dest.x2) / 2, (dest.y1 + dest.y2) / 2, height, height, cam->zoom / 1000, cam->zoom * 1000);
		bg = ColorBackGround3D;
	}else if (type == View2D){
		height = cam->zoom;
		NixSetProjectionOrthoExt((dest.x1 + dest.x2) / 2, (dest.y1 + dest.y2) / 2, height, -height, 0, 1);
	}else{
		height = cam->zoom;
		NixSetProjectionOrthoExt((dest.x1 + dest.x2) / 2, (dest.y1 + dest.y2) / 2, height, -height, - cam->zoom * 1000, cam->zoom * 1000);
	}
	projection = NixProjectionMatrix;

	// background color
	NixSetColor(ColorBackGround3D);
	NixDraw2D(r_id,NixTargetRect,0.9999999f);

	// camera matrix
	vector pos = cam->pos;
	if (type == ViewFront){
		view_kind = _("Vorne");
		ang = - e_y * pi;
	}else if (type == ViewBack){
		view_kind = _("Hinten");
		ang = v_0;
	}else if (type == ViewRight){
		view_kind = _("Rechts");
		ang = - e_y * pi / 2;
	}else if (type == ViewLeft){
		view_kind = _("Links");
		ang = e_y * pi / 2;
	}else if (type == ViewTop){
		view_kind = _("Oben");
		ang = e_x * pi / 2;
	}else if (type == ViewBottom){
		view_kind = _("Unten");
		ang = - e_x * pi / 2;
	}else if (type == ViewPerspective){
		view_kind = _("Perspektive");
		if (!cam->ignore_radius)
			pos -= cam->radius * cam->ang.ang2dir();
		ang = cam->ang;
	}else if (type == ViewIsometric){
		view_kind = _("Isometrisch");
		ang = cam->ang;
	}else if (type == View2D){
		view_kind = _("2D");
		ang = - pi * e_y;
	}
	multi_view->cur_projection_win = this;
	NixSetView(pos, ang);
	mat = NixViewMatrix;
	NixSetZ(true,true);
	NixSetWire(false);
	NixEnableLighting(false);
	if (multi_view->grid_enabled)
		DrawGrid();

	NixSetWire(multi_view->wire_mode);
	// light
	vector dir = -cam->ang.ang2dir();
	color am = color(1,0.3f,0.3f,0.3f);
	color di = color(1,0.6f,0.6f,0.6f);
	color sp = color(1,0.4f,0.4f,0.4f);
	NixSetLightDirectional(multi_view->light,dir,am,di,sp);
	NixEnableLight(multi_view->light, true);
	NixEnableLighting(multi_view->light_enabled);
	NixSetAmbientLight(Black);
	NixSetMaterial(Black,White,Black,0,White);//Black);
	NixSetColor(White);

	// draw the actual data
	//msg_db_r("sub",2);
	if (ed->cur_mode)
		ed->cur_mode->OnDrawWinRecursive(this);
	//msg_db_l(2);

	// draw multiview data
	NixSetShader(-1);
	NixSetAlpha(AlphaNone);
	NixSetTexture(-1);
	NixSetWire(false);
	NixEnableLighting(false);
	foreachi(MultiViewData &d, multi_view->data, di){
		if ((d.Drawable)||(d.Indexable)){
			for (int i=0;i<d.data->num;i++){

				MultiViewSingleData *sd = MVGetSingleData(d, i);
				if (sd->view_stage < multi_view->view_stage)
					continue;

				bool _di = ((d.Indexable) && (sd->is_selected) && (NixGetKey(KEY_I)));
				if ((!d.Drawable) && (!_di))
					continue;
				vector p = Project(sd->pos);
				if ((p.x<dest.x1)||(p.y<dest.y1)||(p.x>dest.x2)||(p.y>dest.y2)||(p.z<=0)||(p.z>=1))
					continue;
				if (_di)
					NixDrawStr(p.x+3, p.y, i2s(i));
				if (d.Drawable){
					color c=Blue;
					float radius = (float)PointRadius;
					float z=0.1f;
					if (sd->is_selected){
						c=Red;
						z=0.05f;
					}
					if (sd->is_special)
						c = Green;
					if ((multi_view->MouseOverSet==di)&&(i==multi_view->MouseOver)){
						c=color(c.a,c.r+0.4f,c.g+0.4f,c.b+0.4f);
						z=0.0f;
						radius = (float)PointRadiusMouseOver;
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

	name_dest = rect(dest.x1 + 3, dest.x1 + 3 + NixGetStrWidth(view_kind), dest.y1, dest.y1 + 20);
	NixSetColor((this == multi_view->active_win) ? ColorText : ColorWindowType);
	if (name_dest.inside(multi_view->m.x, multi_view->m.y))
		NixSetColor(Red);
	ed->DrawStr(dest.x1 + 3, dest.y1, view_kind);
	NixSetColor(ColorText);

	foreach(MultiView::Message3d &m, multi_view->message3d){
		vector p = Project(m.pos);
		if (p.z > 0)
			ed->DrawStr(p.x, p.y, m.str);
	}

	msg_db_l(2);
}

void MultiView::OnDraw()
{
	msg_db_r("Multiview.OnDraw",2);

	update_zoom;

	NixSetZ(true,true);
	NixSetColor(ColorText);



	if (!mode3d){
		win[0].dest = rect(0,MaxX,0,MaxY);
		win[0].Draw();
	}else if (whole_window){
		active_win->dest = rect(0,MaxX,0,MaxY);
		active_win->Draw();
	}else{
		// top left
		win[0].dest = rect(0,MaxX/2,0,MaxY/2);
		win[0].Draw();

		// top right
		win[1].dest = rect(MaxX/2,MaxX,0,MaxY/2);
		win[1].Draw();

		// bottom left
		win[2].dest = rect(0,MaxX/2,MaxY/2,MaxY);
		win[2].Draw();

		// bottom right
		win[3].dest = rect(MaxX/2,MaxX,MaxY/2,MaxY);
		win[3].Draw();

		NixScissor(NixTargetRect);
		NixEnableLighting(false);
		NixSetColor(color(1,0.1f,0.1f,0.5f));
		NixDrawRect(0, MaxX, MaxY/2-1, MaxY/2+2, 0);
		NixDrawRect(MaxX/2-1, MaxX/2+2, 0, MaxY, 0);
	}
	cur_projection_win = NULL;
	NixEnableLighting(false);

	if ((MVRectable)&&(MVRect)){
		NixSetZ(false, false);
		NixSetAlphaM(AlphaMaterial);
		NixSetColor(color(0.2f,0,0,1));
		NixDrawRect(m.x, RectX, m.y, RectY, 0);
		NixSetColor(color(0.7f,0,0,1));
		NixDrawLineV(RectX	,RectY	,m.y	,0);
		NixDrawLineV(m.x	,RectY	,m.y	,0);
		NixDrawLineH(RectX	,m.x	,RectY	,0);
		NixDrawLineH(RectX	,m.x	,m.y	,0);
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

vector MultiViewWindow::Unproject(const vector &p, const vector &o)
{
	vector r;
	vector pp = p;
	if ((type == ViewPerspective) || (type == ViewIsometric)){ // 3D
		if (multi_view->cur_projection_win != this){
			multi_view->cur_projection_win = this;
			NixSetProjectionMatrix(projection);
			NixSetView(mat);
		}
		vector p_o;
		NixGetVecProject(p_o, o);
		pp.z = p_o.z;
		NixGetVecUnproject(r, pp);
	}else if (type == View2D){
		r.x=(p.x-MaxX/2)/cam->zoom+cam->pos.x;
		r.y=(p.y-MaxY/2)/cam->zoom+cam->pos.y;
		r.z=0;
	}else{ // 2D
		r=o;
		if (multi_view->whole_window){
			pp.x-=MaxX/4;
			pp.y-=MaxY/4;
		}else{
			pp.x-=dest.x1;
			pp.y-=dest.y1;
		}
		float zoom = cam->zoom;
		vector &pos = cam->pos;
		if (type == ViewFront){
			r.x=-(pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewBack){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewRight){
			r.z= (pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewLeft){
			r.z=-(pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewTop){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z=-(pp.y-MaxY/4)/zoom+pos.z;
		}else if (type == ViewBottom){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z= (pp.y-MaxY/4)/zoom+pos.z;
		}
	}
	return r;
}

vector MultiViewWindow::Project(const vector &p)
{
	vector r;
	if ((type == ViewPerspective) || (type == ViewIsometric)){ // 3D
		if (multi_view->cur_projection_win != this){
			multi_view->cur_projection_win = this;
			NixSetProjectionMatrix(projection);
			NixSetView(mat);
		}
		NixGetVecProject(r,p);
	}else if (type == View2D){
		r.x=MaxX/2+(p.x-cam->pos.x)*cam->zoom;
		r.y=MaxY/2+(p.y-cam->pos.y)*cam->zoom;
		r.z=0.5f;
	}else{ // 2D
		float zoom = cam->zoom;
		vector &pos = cam->pos;
		if (type == ViewFront){
			r.x=MaxX/4-(p.x-pos.x)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (type == ViewBack){
			r.x=MaxX/4+(p.x-pos.x)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (type == ViewRight){
			r.x=MaxX/4+(p.z-pos.z)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (type == ViewLeft){
			r.x=MaxX/4-(p.z-pos.z)*zoom;
			r.y=MaxY/4-(p.y-pos.y)*zoom;
		}else if (type == ViewTop){
			r.x=MaxX/4+(p.x-pos.x)*zoom;
			r.y=MaxY/4-(p.z-pos.z)*zoom;
		}else if (type == ViewBottom){
			r.x=MaxX/4+(p.x-pos.x)*zoom;
			r.y=MaxY/4+(p.z-pos.z)*zoom;
		}
		if (multi_view->whole_window){
			r.x+=MaxX/4;
			r.y+=MaxY/4;
		}else{
			r.x+=dest.x1;
			r.y+=dest.y1;
		}
		r.z=0.5f+VecDotProduct(p-pos,GetDirection())/cam->radius/32;
	}
	return r;
}

vector MultiViewWindow::Unproject(const vector &p)
{
	vector r;
	vector pp = p;
	if ((type == ViewPerspective) || (type == ViewIsometric)){ // 3D
		if (multi_view->cur_projection_win != this){
			multi_view->cur_projection_win = this;
			NixSetProjectionMatrix(projection);
			NixSetView(mat);
		}
		NixGetVecUnproject(r,pp);
	}else if (type == View2D){
		r.x=(pp.x-MaxX/2)/cam->zoom+cam->pos.x;
		r.y=(pp.y-MaxY/2)/cam->zoom+cam->pos.y;
		r.z=0;
	}else{ // 2D
		if (multi_view->whole_window){
			pp.x-=MaxX/4;
			pp.y-=MaxY/4;
		}else{
			pp.x-=dest.x1;
			pp.y-=dest.y1;
		}
		r=cam->pos;
		float zoom = cam->zoom;
		vector &pos = cam->pos;
		if (type == ViewFront){
			r.x=-(pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewBack){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewRight){
			r.z= (pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewLeft){
			r.z=-(pp.x-MaxX/4)/zoom+pos.z;
			r.y=-(pp.y-MaxY/4)/zoom+pos.y;
		}else if (type == ViewTop){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z=-(pp.y-MaxY/4)/zoom+pos.z;
		}else if (type == ViewBottom){
			r.x= (pp.x-MaxX/4)/zoom+pos.x;
			r.z= (pp.y-MaxY/4)/zoom+pos.z;
		}
	}
	return r;
}

vector MultiViewWindow::GetDirection()
{
	int t=type;
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
		return cam->ang.ang2dir();
	return v_0;
}

vector MultiViewWindow::GetDirectionUp()
{
	int t=type;
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
		return VecAngAdd(vector(-pi/2,0,0), cam->ang).ang2dir();
	return v_0;
}

vector MultiViewWindow::GetDirectionRight()
{
	vector d=GetDirection();
	vector u=GetDirectionUp();
	return VecCrossProduct(d,u);
}

void MultiViewWindow::GetMovingFrame(vector &dir, vector &up, vector &right)
{
	dir = GetDirection();
	up = GetDirectionUp();
	right = dir ^ up;
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
	return mouse_win->Unproject(m, cam.pos);
}

vector MultiView::GetCursor3d(const vector &depth_reference)
{
	return mouse_win->Unproject(m, depth_reference);
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
					vector p = mouse_win->Project(sd->pos);
					if ((p.z<=0)||(p.z>=1))
						continue;
					mo=((m.x>=p.x-_radius)&&(m.x<=p.x+_radius)&&(m.y>=p.y-_radius)&&(m.y<=p.y+_radius));
					if (mo){
						mop=sd->pos;
						z_min=0;
					}
				}
				if ((!mo)&&(d.IsMouseOver)){
					vector tp;
					mo=d.IsMouseOver(i, d.user_data, mouse_win, tp);
					if (mo){
						float z = mouse_win->Project(tp).z;
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

	rect r = rect(min(m.x, RectX), max(m.x, RectX), min(m.y, RectY), max(m.y, RectY));

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
					sd->m_delta = d.IsInRect(i, d.user_data, active_win, &r);
				}else{// if (!sd->m_delta){
					vector p = active_win->Project(sd->pos);
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
	HoldingX = m.x;
	HoldingY = m.y;
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
		vector d, u, r;
		mouse_win->GetMovingFrame(d, u, r);
		cur_action = ActionMultiViewFactory(action[button].name, _data_, mouse_action_param, mouse_action_pos0,
				r, u, d);
		cur_action->execute_logged(_data_);
		Notify("ActionStart");
	}
}


vector transform_ang(MultiView *mv, const vector &ang)
{
	quaternion qmv, mqmv, qang, q;
	QuaternionRotationV(qmv,  mv->active_win->ang);
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

		vector v2p = m;
		vector v2  = active_win->Unproject(v2p, mouse_action_pos0);
		vector v1  = mouse_action_pos0;
		vector v1p = active_win->Project(v1);
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
			mouse_action_param = active_win->GetDirectionRight();
		else
			mouse_action_param = v_0;
		cur_action->undo(_data_);
		delete(cur_action);
		vector d, u, r;
		active_win->GetMovingFrame(d, u, r);
		cur_action = ActionMultiViewFactory(action[active_mouse_action].name, _data_, mouse_action_param, mouse_action_pos0,
				r, u, d);
		cur_action->execute_logged(_data_);

		Notify("ActionUpdate");
	}
}



void MultiView::MouseActionEnd(bool set)
{
	if (cur_action){
		msg_write("mouse action end");
		if (set){
			vector d, u, r;
			active_win->GetMovingFrame(d, u, r);
			cur_action->undo(_data_);
					delete(cur_action);
			cur_action = ActionMultiViewFactory(action[active_mouse_action].name, _data_, mouse_action_param, mouse_action_pos0,
					r, u, d);
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
