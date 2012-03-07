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
							zoom = (float)NixScreenHeight * 0.8f;
#define MVGetSingleData(d, index)	((MultiViewSingleData*) ((char*)(d).data + (d).DataSingleSize * index))

bool pos_in_irect(int x, int y, irect r)
{
	return ((x >= r.x1) and (x <= r.x2) && (y >= r.y1) and (y <= r.y2));
}

MultiView::MultiView(bool _mode3d)
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

	Reset();
}

MultiView::~MultiView()
{
}

void MultiView::Reset()
{
	pos = v0;
	ang = v0;
	radius = 100;
	zoom = 1;
	whole_window = false;
	grid_enabled = true;
	light_enabled = true;
	ignore_radius = false;
	wire_mode = false;
	MVRectable = MVRect = false;
	mouse_win = 0;

	ViewMoving = -1;

	view_stage = 0;
}

void MultiView::ResetData()
{
	data.clear();
}

void MultiView::SetData(int type, const DynamicArray & a, void *user_data, int mode, t_is_mouse_over_func *is_mouse_over_func, t_is_in_rect_func *is_in_rect_func)
{
	MultiViewData d;

	d.Num = a.num;
	d.Type = type;
	d.data = a.data;
	d.user_data = user_data;
	d.MVSelectable = (mode & FlagSelect)>0;
	d.Drawable = (mode & FlagDraw)>0;
	d.Indexable = (mode & FlagIndex)>0;
	d.Movable = (mode & FlagMove)>0;
	d.IsMouseOver = is_mouse_over_func;
	d.IsInRect = is_in_rect_func;
	d.DataSingleSize = a.element_size;
	data.add(d);
}



void MultiView::DoZoom(float factor)
{
	vector mup;
	if (!mode3d){
		mup = VecUnProject(vector((float)mx,(float)my,0),mouse_win);
		zoom *= factor;
		pos += mup - VecUnProject(vector((float)mx,(float)my,0),mouse_win);
	}else{
		if (view[mouse_win].type != ViewPerspective)
			mup = VecUnProject(vector((float)mx,(float)my,0),mouse_win);
		radius /= factor;
		update_zoom;
		if (view[mouse_win].type != ViewPerspective)
			pos += mup - VecUnProject(vector((float)mx,(float)my,0),mouse_win);
	}
	ed->ForceRedraw();
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
	ed->ForceRedraw();
}

void MultiView::ToggleWholeWindow()
{
	whole_window = !whole_window;
	view[4].type = view[mouse_win].type;
	ed->ForceRedraw();
}

void MultiView::ToggleGrid()
{
	grid_enabled = !grid_enabled;
	ed->ForceRedraw();
}

void MultiView::ToggleLight()
{
	light_enabled = !light_enabled;
	ed->ForceRedraw();
}

void MultiView::ToggleWire()
{
	wire_mode = !wire_mode;
	ed->ForceRedraw();
}

void MultiView::OnCommand(const string & id)
{
	HuiEvent *e = HuiGetEvent();

	//msg_write(id);

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
}



void MultiView::OnLeftButtonDown()
{
	// menu for selection of view type
	if (pos_in_irect(mx, my, view[mouse_win].name_dest)){
		menu->OpenPopup(ed->win, mx, my);
		return;
	}

	MouseMovedSinceClick = 0;
	Moved = false;
	vx = vy = 0;
	GetSelected();

	if (Selected<0){
		if (MVRectable)
			StartRect();
	}else{
		MovingWin = mouse_win;
		MovingStart = MouseOverTP;
		MovingDP = MVGetSingleData(data[SelectedSet], Selected)->pos - MouseOverTP;
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
	}
}



void MultiView::OnMiddleButtonUp()
{
	if (ViewMoving >= 0){
		ViewMoving = -1;
		HoldCursor(false);
	}
}



void MultiView::OnRightButtonUp()
{
	if (ViewMoving >= 0){
		ViewMoving = -1;
		HoldCursor(false);
	}
}



void MultiView::OnKeyUp()
{
}



void MultiView::OnLeftButtonUp()
{
	EndRect();
	MultiViewEditing=false;
}



void MultiView::OnMouseMove()
{
	mx = HuiGetEvent()->mx;
	my = HuiGetEvent()->my - 27;
	vx = HuiGetEvent()->dx;
	vy = HuiGetEvent()->dy;
	msg_todo("hui liefert falsche Maus");


	// which window is the cursor in?
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


	// hover
	if ((!NixGetButton(0)) && (!NixGetButton(1)) && (!NixGetButton(2)))
		GetMouseOver();

	if (MVRect)
		SelectAllInRectangle();

	// left button -> move data
	/*if (NixGetButton(0)){
		MouseMovedSinceClick += abs(vx) + abs(vy);
		if ( MouseMovedSinceClick >= MinMouseMoveToInteract ){
			MultiViewEditing = true;
			if (Selected >= 0)
				MoveSelection();
		}
	}*/


	if (ViewMoving >= 0){
		int t = view[ViewMoving].type;
		if ((t == ViewPerspective) || (t == ViewIsometric)){
// camera rotation
			bool RotatingOwn = (NixGetButton(1) || (NixGetKey(KEY_CONTROL)));
			if (RotatingOwn)
				pos -= radius * VecAng2Dir(ang);
			vector dang = vector((float)vy, (float)vx, 0) * MouseRotationSpeed;
			ang = VecAngAdd(dang, ang);
			if (RotatingOwn)
				pos += radius * VecAng2Dir(ang);
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
//		ed->win->SetCursorPos(mx - vx, my - vy);
		// evil hack... nix doesn't get nur pos data...
		/*NixInputDataCurrent.x -= vx;
		NixInputDataCurrent.y -= vy;
		NixInputDataCurrent.dx = 0;
		NixInputDataCurrent.dy = 0;*/
	}

	ed->ForceRedraw();
}




void MultiView::StartRect()
{
	MVRect=true;
	RectWin=mouse_win;
	RectX=mx;
	RectY=my;

	// reset selection data
	foreach(data, d)
		if (d.MVSelectable)
			for (int i=0;i<d.Num;i++){
				MultiViewSingleData* sd = MVGetSingleData(d,i);
				sd->m_old = sd->is_selected;
			}

	ed->ForceRedraw();
}

void MultiView::EndRect()
{
	/*if (PostEndRect)
		PostEndRect();*/

	MVRect=false;
	RectWin=-1;

	ed->ForceRedraw();
}


#define GridConst	5.0f

float GetGridD(float zoom)
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


void MultiView::DrawGrid(int win, irect dest)
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
		vector PerspectiveViewPos = radius * VecAng2Dir(ang) - pos;
		//NixSetZ(false,false);
		// horizontale
		float r=NixMaxDepth*0.6f;
		for (int j=-16;j<16;j++)
			for (int i=0;i<64;i++){
				vector pa=VecAng2Dir(vector(float(j)/32*pi,float(i  )/32*pi,0))*r-PerspectiveViewPos;
				vector pb=VecAng2Dir(vector(float(j)/32*pi,float(i+1)/32*pi,0))*r-PerspectiveViewPos;
				color c=ColorInterpolate(ColorBackGround2D,ColorGrid,j==0?0.6f:0.1f);
				NixDrawLine3D(pa,pb,c);
			}
		// vertikale
		for (int j=0;j<32;j++)
			for (int i=0;i<64;i++){
				vector pa=VecAng2Dir(vector(float(i  )/32*pi,float(j)/32*pi,0))*r-PerspectiveViewPos;
				vector pb=VecAng2Dir(vector(float(i+1)/32*pi,float(j)/32*pi,0))*r-PerspectiveViewPos;
				color c=ColorInterpolate(ColorBackGround2D,ColorGrid,(j%16)==0?0.6f:0.1f);
				NixDrawLine3D(pa,pb,c);
			}
		//NixSetZ(true,true);
		return;
	}

	// rectangular
	float D = GetGridD(zoom);
	int a,b;
	float fa,fb,t;

	vector vux1 = VecUnProject(vector((float)dest.x1,0,0), win);
	vector vux2 = VecUnProject(vector((float)dest.x2,0,0), win);
	vector vuy1 = VecUnProject(vector(0,(float)dest.y1,0), win);
	vector vuy2 = VecUnProject(vector(0,(float)dest.y2,0), win);
	vector n,va,vb;

	// vertikal
	n=vux2-vux1;
	n/=VecLengthFuzzy(n);	//VecNormalize(n);
	va=n*VecDotProduct(n,vux1);
	vb=n*VecDotProduct(n,vux2);
	fa=(va.x+va.y+va.z)/D;
	fb=(vb.x+vb.y+vb.z)/D;
	if (fa>fb){	t=fa;	fa=fb;	fb=t;	}
	a=(int)fa;
	b=(int)fb+1;
	for (int i=a;i<b;i++){
		int x=(int)VecProject(vector((float)i*D,(float)i*D,(float)i*D),win).x;
		color c=ColorInterpolate(ColorBackGround2D,ColorGrid,GetDensity(i,(float)MaxX/(fb-fa)));
		NixDrawLineV(x,dest.y1,dest.y2,c,0.99998f-GetDensity(i,(float)MaxX/(fb-fa))*0.00005f);
	}

	// horizontal
	n=vuy2-vuy1;
	n/=VecLengthFuzzy(n);	//-VecNormalize(n);
	va=n*VecDotProduct(n,vuy1);
	vb=n*VecDotProduct(n,vuy2);
	fa=(va.x+va.y+va.z)/D;
	fb=(vb.x+vb.y+vb.z)/D;
	if (fa>fb){	t=fa;	fa=fb;	fb=t;	}
	a=(int)fa;
	b=(int)fb+1;
	for (int i=a;i<b;i++){
		int y=(int)VecProject(vector((float)i*D,(float)i*D,(float)i*D),win).y;
		color c=ColorInterpolate(ColorBackGround2D,ColorGrid,GetDensity(i,(float)MaxX/(fb-fa)));
		NixDrawLineH(dest.x1,dest.x2,y,c,0.99998f-GetDensity(i,(float)MaxX/(fb-fa))*0.00005f);
	}
}


void MultiView::DrawMousePos()
{
	vector m = GetCursor3d();
	float l = GetGridD(zoom) * 10.1f;
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
	if (l>0.000001f){				unit = "u";	f=1000000.0f;	}
	if (l>0.001f){					unit = "m";	f=1000.0f;	}
	if (l>1.0f){					unit = "";	f=1.0f;	}
	if (l>1000.0f){					unit = "k";	f=0.001f;	}
	if (l>1000000.0f){				unit = "M";	f=0.000001f;	}
	if (l>1000000000.0f){			unit = "G";	f=0.000000001f;	}
	if (l>1000000000000.0f){		unit = "T";	f=0.000000000001f;	}
	if (l>1000000000000000.0f){		unit = "P";	f=0.000000000000001f;	}
	//if (l>1000000000000000000.0f){
	//}
	m *= f;
	string sx = f2s(m.x,2) + " " + unit;
	string sy = f2s(m.y,2) + " " + unit;
	string sz = f2s(m.z,2) + " " + unit;
	int slx = NixGetStrWidth(sx, -1, -1);
	int sly = NixGetStrWidth(sy, -1, -1);
	int slz = NixGetStrWidth(sz, -1, -1);

	if (view[mouse_win].type == View2D){
		ed->DrawStr(MaxX - slx, MaxY - 60, sx);
		ed->DrawStr(MaxX - sly, MaxY - 40, sy);
	}else{
		ed->DrawStr(MaxX - slx, MaxY - 80, sx);
		ed->DrawStr(MaxX - sly, MaxY - 60, sy);
		ed->DrawStr(MaxX - slz, MaxY - 40, sz);
	}
}


void MultiView::DrawWin(int win, irect dest)
{
	msg_db_r("MultiView.DrawWin",2);
	matrix r,s,t;
	view[win].dest = dest;
	NixStartPart(dest.x1, dest.y1, dest.x2, dest.y2, true);
	string view_kind;
	MatrixIdentity(view[win].mat);

	if (view[win].type == ViewPerspective){
		if (whole_window)
			NixSetPerspectiveMode(PerspectiveSizeSet,(float)NixScreenWidth,(float)NixScreenHeight);
		else
			NixSetPerspectiveMode(PerspectiveSizeSet,(float)NixScreenWidth/2,(float)NixScreenHeight/2);
		NixDraw2D(-1,ColorBackGround3D,r01,NixTargetRect,0.9999999f);
	}else if (view[win].type == View2D){
		NixSetPerspectiveMode(Perspective2DScaleSet, zoom, zoom);
		//NixDraw2D(-1,ColorBackGround2D,r01,NixTargetRect,0.9999999f);
	}else{
		NixSetPerspectiveMode(Perspective2DScaleSet, zoom, zoom);
		//NixSetPerspectiveMode(Perspective2DScaleSet,1,1);
		NixDraw2D(-1,ColorBackGround2D,r01,NixTargetRect,0.9999999f);
	}

	// camera position
	vector vt=-pos;
	MatrixTranslation(t,vt);
	if (view[win].type == ViewFront){
		view_kind = _("Vorne");
		MatrixRotationY(r,pi);
	}else if (view[win].type == ViewBack){
		view_kind = _("Hinten");
		MatrixIdentity(r);
	}else if (view[win].type == ViewRight){
		view_kind = _("Rechts");
		MatrixRotationY(r,pi/2);
	}else if (view[win].type == ViewLeft){
		view_kind = _("Links");
		MatrixRotationY(r,-pi/2);
	}else if (view[win].type == ViewTop){
		view_kind = _("Oben");
		MatrixRotationX(r,-pi/2);
	}else if (view[win].type == ViewBottom){
		view_kind = _("Unten");
		MatrixRotationX(r,pi/2);
	}else if (view[win].type == ViewPerspective){
		view_kind = _("Perspektive");
		float _radius=ignore_radius ? 0 : radius;
		vector PerspectiveViewPos=_radius*VecAng2Dir(ang)+vt;
		MatrixTranslation(t,PerspectiveViewPos);
		MatrixRotationView(r,ang);
	}else if (view[win].type == ViewIsometric){
		view_kind = _("Isometrisch");
		float _radius = ignore_radius? 0 : radius;
		vector PerspectiveViewPos=_radius*VecAng2Dir(ang)+vt;
		MatrixTranslation(t,PerspectiveViewPos);
		MatrixRotationView(r,ang);
	}else if (view[win].type == View2D){
		view_kind = _("2D");
		vt=-pos;
		MatrixTranslation(t,vt);
		MatrixRotationY(r,pi);
	}
	MatrixMultiply(view[win].mat,r,t);
	cur_view = cur_view_rect = win;
	NixSetViewM((view[win].type == ViewPerspective), view[win].mat);
	//NixSetView(true,vector(0,0,-200),vector(0,0,0));
	NixSetZ(true,true);
	NixSetWire(false);
	DrawGrid(win,dest);

	NixSetWire(wire_mode);
	NixEnableLighting(light_enabled);
	NixSetMaterial(Black,White,Black,0,Black);

	// draw the actual data
	//msg_db_r("sub",2);
	if (ed->cur_mode)
		ed->cur_mode->DrawWin(win, dest);
	if (ed->creation_mode)
		ed->creation_mode->PostDrawWin(win, dest);
	//msg_db_l(2);

	// draw multiview data
	NixSetWire(false);
	foreachi(data, d, di){
		if ((d.Drawable)||(d.Indexable)){
			for (int i=0;i<d.Num;i++){

				MultiViewSingleData *sd = MVGetSingleData(d, i);
				if (sd->view_stage < view_stage)
					continue;

				bool di = ((d.Indexable) && (sd->is_selected) && (NixGetKey(KEY_I)));
				if ((!d.Drawable) && (!di))
					continue;
				vector p = VecProject(sd->pos,win);
				if ((p.x<dest.x1)||(p.y<dest.y1)||(p.x>dest.x2)||(p.y>dest.y2)||(p.z<=0)||(p.z>=1))
					continue;
				if (di)
					NixDrawInt((int)p.x+3,(int)p.y,i);
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
					NixDrawRect(	p.x-radius,
									p.x+radius,
									p.y-radius,
									p.y+radius,
									c,z);
				}
			}
		}
	}

	// DEBUG!
	//NixSetZ(false,false);
	//NixDrawLine3D(MouseOverTP,MouseOverTP+vector(0,ViewRadius/3,0),Green);

	// type of view

	view[win].name_dest = irect(dest.x1 + 3, dest.x1 + 3 + NixGetStrWidth(view_kind, -1, -1), dest.y1, dest.y1 + 20);
	if (pos_in_irect(mx, my, view[win].name_dest))
		NixSetFontColor(Red);
	ed->DrawStr(dest.x1 + 3, dest.y1, view_kind);
	NixSetFontColor(ColorText);
	msg_db_l(2);
}

void MultiView::Draw()
{
	msg_db_r("All::DrawMultiview",2);
	NixMaxDepth = radius * 1000;
	NixMinDepth = radius / 1000;

	update_zoom;

// light
	vector dir=-VecAng2Dir(ang);
	color am=color(1,0.3f,0.3f,0.3f);
	color di=color(1,0.6f,0.6f,0.6f);
	color sp=color(1,0.4f,0.4f,0.4f);
	NixSetLightDirectional(light,dir,am,di,sp);
	NixEnableLight(light,true);
	NixSetAmbientLight(Black);
	NixSetZ(true,true);
	NixSetFontColor(ColorText);

	if (!mode3d)
		DrawWin(0,irect(0,MaxX,0,MaxY));
	else if (whole_window)
		DrawWin(4,irect(0,MaxX,0,MaxY));
	else{
		// top left
		DrawWin(0,irect(0,MaxX/2,0,MaxY/2));

		// top right
		DrawWin(1,irect(MaxX/2,MaxX,0,MaxY/2));

		// bottom left
		DrawWin(2,irect(0,MaxX/2,MaxY/2,MaxY));

		// bottom right
		DrawWin(3,irect(MaxX/2,MaxX,MaxY/2,MaxY));

		NixStartPart(-1,-1,-1,-1,true);
		color c = color(1,0.1f,0.1f,0.5f);
		NixDrawRect(0, MaxX, MaxY/2-1, MaxY/2+2, c, 0);
		NixDrawRect(MaxX/2-1, MaxX/2+2, 0, MaxY, c, 0);
	}
	cur_view = -1;
	cur_view_rect = -1;

	if ((MVRectable)&&(MVRect)){
		NixSetZ(false, false);
		NixSetAlphaM(AlphaMaterial);
		color c=color(0.2f,0,0,1);
		NixDrawRect((float)mx,(float)RectX,(float)my,(float)RectY,c,0);
		c=color(0.7f,0,0,1);
		NixDrawLineV(RectX	,RectY	,my		,c,0);
		NixDrawLineV(mx		,RectY	,my		,c,0);
		NixDrawLineH(RectX	,mx		,RectY	,c,0);
		NixDrawLineH(RectX	,mx		,my		,c,0);
		NixSetAlphaM(AlphaNone);
		NixSetZ(true, true);
	}

	DrawMousePos();

	/*if (PostDrawMultiView)
		PostDrawMultiView();*/

	msg_db_l(2);
}

vector MultiView::VecUnProject2(vector p,vector o,int win)
{
	vector r;
	int t=view[win].type;
	if ((t==ViewPerspective) || (t==ViewIsometric)){ // 3D
		if (cur_view!=win){
			cur_view=win;
			//NixSetView(true,ViewMatrix[win]);
		}
		if ((cur_view_rect!=win)&&(!whole_window)){
			p.x-=(view[win].dest.x1+view[win].dest.x2-MaxX)/2;
			p.y-=(view[win].dest.y1+view[win].dest.y2-MaxY)/2;
		}
		vector p_o;
		NixGetVecProject(p_o,o);
		p.z=p_o.z;
		NixGetVecUnproject(r,p);
	}else if (t==View2D){
		r.x=(p.x-MaxX/2)/zoom+pos.x;
		r.y=(p.y-MaxY/2)/zoom+pos.y;
		r.z=0;
	}else{ // 2D
		r=o;
		if (whole_window){
			p.x-=MaxX/4;
			p.y-=MaxY/4;
		}else{
			p.x-=MaxX/2*float(win%2);
			p.y-=MaxY/2*float(win/2);
		}
		if (t==ViewFront){
			r.x=-(p.x-MaxX/4)/zoom+pos.x;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewBack){
			r.x= (p.x-MaxX/4)/zoom+pos.x;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewRight){
			r.z= (p.x-MaxX/4)/zoom+pos.z;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewLeft){
			r.z=-(p.x-MaxX/4)/zoom+pos.z;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewTop){
			r.x= (p.x-MaxX/4)/zoom+pos.x;
			r.z=-(p.y-MaxY/4)/zoom+pos.z;
		}else if (t==ViewBottom){
			r.x= (p.x-MaxX/4)/zoom+pos.x;
			r.z= (p.y-MaxY/4)/zoom+pos.z;
		}
	}
	return r;
}

vector MultiView::VecProject(vector p,int win)
{
	vector r;
	int t=view[win].type;
	if ((t==ViewPerspective) || (t==ViewIsometric)){ // 3D
		if (cur_view!=win){
			cur_view=win;
			//NixSetView(true,ViewMatrix[win]);
		}
		NixGetVecProject(r,p);
		if ((cur_view_rect!=win)&&(!whole_window)){
			r.x+=(view[win].dest.x1+view[win].dest.x2-MaxX)/2;
			r.y+=(view[win].dest.y1+view[win].dest.y2-MaxY)/2;
		}
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

vector MultiView::VecUnProject(vector p,int win)
{
	vector r;
	int t=view[win].type;
	if ((t==ViewPerspective) || (t==ViewIsometric)){ // 3D
		if (cur_view!=win){
			cur_view=win;
			//NixSetView(true,ViewMatrix[win]);
		}
		if ((cur_view_rect!=win)&&(!whole_window)){
			p.x-=(view[win].dest.x1+view[win].dest.x2-MaxX)/2;
			p.y-=(view[win].dest.y1+view[win].dest.y2-MaxY)/2;
		}
		NixGetVecUnproject(r,p);
	}else if (t==View2D){
		r.x=(p.x-MaxX/2)/zoom+pos.x;
		r.y=(p.y-MaxY/2)/zoom+pos.y;
		r.z=0;
	}else{ // 2D
		if (whole_window){
			p.x-=MaxX/4;
			p.y-=MaxY/4;
		}else{
			p.x-=MaxX/2*float(win%2);
			p.y-=MaxY/2*float(win/2);
		}
		r=pos;
		if (t==ViewFront){
			r.x=-(p.x-MaxX/4)/zoom+pos.x;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewBack){
			r.x= (p.x-MaxX/4)/zoom+pos.x;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewRight){
			r.z= (p.x-MaxX/4)/zoom+pos.z;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewLeft){
			r.z=-(p.x-MaxX/4)/zoom+pos.z;
			r.y=-(p.y-MaxY/4)/zoom+pos.y;
		}else if (t==ViewTop){
			r.x= (p.x-MaxX/4)/zoom+pos.x;
			r.z=-(p.y-MaxY/4)/zoom+pos.z;
		}else if (t==ViewBottom){
			r.x= (p.x-MaxX/4)/zoom+pos.x;
			r.z= (p.y-MaxY/4)/zoom+pos.z;
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
		return VecAng2Dir(ang);
	return v0;
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
		return VecAng2Dir(VecAngAdd(vector(-pi/2,0,0), ang));
	return v0;
}

vector MultiView::GetDirectionRight(int win)
{
	vector d=GetDirection(win);
	vector u=GetDirectionUp(win);
	return VecCrossProduct(d,u);
}

vector MultiView::GetCursor3d()
{
	//return VecUnProject(vector((float)mx,(float)my,0), mouse_win);
	return VecUnProject2(vector((float)mx, (float)my, 0), pos, mouse_win);
}


void MultiView::GetMouseOver()
{
	msg_db_r("GetMouseOver",6);
	MouseOver=MouseOverType=MouseOverSet=-1;
	/*if (!MVSelectable)
		return;*/
	float _radius=(float)PointRadiusMouseOver;
	float z_min=1;
	foreachi(data, d, di)
		if (d.MVSelectable)
			for (int i=0;i<d.Num;i++){
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
	foreach(data, d)
		if (d.MVSelectable)
			for (int i=0;i<d.Num;i++){
				MultiViewSingleData* sd = MVGetSingleData(d,i);
				sd->is_selected = false;
			}
	MultiViewSelectionChanged = true;
}

void MultiView::GetSelected()
{
	msg_db_r("GetSelected",4);
	Selected=MouseOver;
	SelectedType=MouseOverType;
	SelectedSet=MouseOverSet;
	SelectedTP=MouseOverTP;
	if ((Selected<0)||(SelectedType<0)){
		if ((!NixGetKey(KEY_CONTROL))&&(!NixGetKey(KEY_SHIFT)))
			UnselectAll();
	}else{
		MultiViewSingleData* sd=MVGetSingleData(data[SelectedSet],Selected);
		if (sd->is_selected){
			if (NixGetKey(KEY_SHIFT)){
				sd->is_selected=false;
				Selected=SelectedType=-1;
			}
		}else{
			if ((NixGetKey(KEY_CONTROL))||(NixGetKey(KEY_SHIFT)))
				sd->is_selected=true;
			else{
				UnselectAll();
				sd->is_selected=true;
			}
		}
	}
	MultiViewSelectionChanged=true;
	msg_db_l(4);
}

void MultiView::SelectAllInRectangle()
{
	msg_db_r("SelAllInRect",4);
	int x1=RectX,y1=RectY,x2=mx,y2=my,a;
	// reset data
	UnselectAll();
	// normalize rectangle
	if (x2<x1){		a=x2;	x2=x1;	x1=a;	}
	if (y2<y1){		a=y2;	y2=y1;	y1=a;	}
	irect r=irect(x1,x2,y1,y2);

	// select
	foreach(data, d)
		if (d.MVSelectable)
			for (int i=0;i<d.Num;i++){
				MultiViewSingleData* sd=MVGetSingleData(d,i);
				if (sd->view_stage<view_stage)
					continue;

				// selected?
				sd->m_delta=false;
				if (d.IsInRect)
					sd->m_delta=d.IsInRect(i,d.user_data, RectWin,&r);
				else{// if (!sd->m_delta){
					vector p=VecProject(sd->pos,RectWin);
					sd->m_delta=((x2>=p.x)&&(x1<=p.x)&&(y2>=p.y)&&(y1<=p.y)&&(p.z>0)&&(p.z<1));
				}

				// add the selection layers
				if (NixGetKey(KEY_SHIFT))
					sd->is_selected=(sd->m_old && !sd->m_delta) || (!sd->m_old && sd->m_delta);
				else
					if (NixGetKey(KEY_CONTROL))
						sd->is_selected=(sd->m_old || sd->m_delta);
					else
						sd->is_selected=sd->m_delta;
			}
	MultiViewSelectionChanged=true;
	msg_db_l(4);
}


void MultiView::HoldCursor(bool holding)
{
	HoldingCursor = holding;
	ed->win->ShowCursor(!holding);
}
