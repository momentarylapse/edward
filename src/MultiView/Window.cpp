/*
 * Window.cpp
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#include "Window.h"
#include "MultiViewImpl.h"
#include "ActionController.h"
#include "SingleData.h"
#include "../Edward.h"


extern matrix NixViewMatrix;
extern matrix NixProjectionMatrix;

namespace MultiView{


#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))


float GetDensity(int i,float t)
{
	if (i%10==0)	t*=10;
	if (i%100==0)	t*=10;
	t=(float)sqrt(t)/60;
	if (t>0.6f)		t=0.6f;
	if (i==0)		t=1;
	return t;
}

Window::Window(MultiViewImpl *_impl, int _type)
{
	impl = _impl;
	multi_view = impl;
	cam = &impl->cam;
	type = _type;
}

void Window::DrawGrid()
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
		vector PerspectiveViewPos = cam->radius * (cam->ang * e_z) - cam->pos;
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
	float D = impl->GetGridD();
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


void Window::Draw()
{
	msg_db_f("MultiView.DrawWin",2);
	matrix rot, trans;
	NixScissor(dest);
	string view_kind;
	MatrixIdentity(mat);
	NixEnableLighting(false);
	NixSetTexture(NULL);

	color bg = ColorBackGround2D;
	float height = NixScreenHeight;
	if (!impl->whole_window)
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
		NixSetProjectionOrthoExt((dest.x1 + dest.x2) / 2, (dest.y1 + dest.y2) / 2, height, -height, - cam->radius * 100, cam->radius * 100);
	}
	projection = NixProjectionMatrix;

	// background color
	NixSetColor(ColorBackGround3D);
	NixDraw2D(r_id,NixTargetRect,0.9999999f);

	// camera matrix
	vector pos = cam->pos;
	if (type == ViewFront){
		view_kind = _("Vorne");
		QuaternionRotationA(ang, e_y, -pi);
	}else if (type == ViewBack){
		view_kind = _("Hinten");
		ang = q_id;
	}else if (type == ViewRight){
		view_kind = _("Rechts");
		QuaternionRotationA(ang, e_y, -pi/2);
	}else if (type == ViewLeft){
		view_kind = _("Links");
		QuaternionRotationA(ang, e_y, pi/2);
	}else if (type == ViewTop){
		view_kind = _("Oben");
		QuaternionRotationA(ang, e_x, pi/2);
	}else if (type == ViewBottom){
		view_kind = _("Unten");
		QuaternionRotationA(ang, e_x, -pi/2);
	}else if (type == ViewPerspective){
		view_kind = _("Perspektive");
		if (!cam->ignore_radius)
			pos -= cam->radius * (cam->ang * e_z);
		ang = cam->ang;
	}else if (type == ViewIsometric){
		view_kind = _("Isometrisch");
		ang = cam->ang;
	}else if (type == View2D){
		view_kind = _("2D");
		QuaternionRotationA(ang, e_y, -pi);
	}
	impl->cur_projection_win = this;
	NixSetView(pos, ang.get_angles());
	mat = NixViewMatrix;
	NixSetZ(true,true);
	NixSetWire(false);
	NixEnableLighting(false);
	NixEnableFog(false);
	NixSetFog(FogExp, 0, 1000, 0, Black); // some shaders need correct fog values
	if (impl->grid_enabled)
		DrawGrid();

	NixSetWire(impl->wire_mode);
	// light
	vector dir = -(cam->ang * e_z);
	color am = color(1,0.3f,0.3f,0.3f);
	color di = color(1,0.6f,0.6f,0.6f);
	color sp = color(1,0.4f,0.4f,0.4f);
	NixSetLightDirectional(impl->light,dir,am,di,sp);
	NixEnableLight(impl->light, true);
	NixEnableLighting(impl->light_enabled);
	NixSetAmbientLight(Black);
	NixSetMaterial(Black,White,Black,0,White);//Black);
	NixSetColor(White);

	// draw the actual data
	//msg_db_r("sub",2);
	if (ed->cur_mode)
		ed->cur_mode->OnDrawWinRecursive(this);
	//msg_db_l(2);

	// draw multiview data
	NixSetShader(NULL);
	NixSetAlpha(AlphaNone);
	NixSetTexture(NULL);
	NixSetWire(false);
	NixEnableLighting(false);
	foreachi(DataSet &d, impl->data, di){
		if ((d.Drawable)||(d.Indexable)){
			for (int i=0;i<d.data->num;i++){

				SingleData *sd = MVGetSingleData(d, i);
				if (sd->view_stage < impl->view_stage)
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
					color c = ColorPoint;
					float radius = (float)PointRadius;
					float z = 0.1f;
					if (sd->is_selected){
						c = ColorPointSelected;
						z = 0.05f;
					}
					if (sd->is_special)
						c = ColorPointSpecial;
					if ((impl->hover.set == di) && (i == impl->hover.index)){
						c = color(c.a,c.r+0.4f,c.g+0.4f,c.b+0.4f);
						z = 0.0f;
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

	if (impl->action_con->show)
		impl->action_con->Draw(this);

	name_dest = rect(dest.x1 + 3, dest.x1 + 3 + NixGetStrWidth(view_kind), dest.y1, dest.y1 + 20);

	NixSetColor(ColorWindowType);
	if (ed->IsActive("nix-area") && (this == impl->active_win))
		NixSetColor(ColorText);
	if (name_dest.inside(impl->m.x, impl->m.y))
		NixSetColor(Red);
	ed->DrawStr(dest.x1 + 3, dest.y1, view_kind);
	NixSetColor(ColorText);

	foreach(MultiViewImpl::Message3d &m, impl->message3d){
		vector p = Project(m.pos);
		if (p.z > 0)
			ed->DrawStr(p.x, p.y, m.str);
	}
}

vector Window::Unproject(const vector &p, const vector &o)
{
	vector r;
	vector pp = p;
	if ((type == ViewPerspective) || (type == ViewIsometric)){ // 3D
		if (impl->cur_projection_win != this){
			impl->cur_projection_win = this;
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
		if (impl->whole_window){
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

vector Window::Project(const vector &p)
{
	vector r;
	if ((type == ViewPerspective) || (type == ViewIsometric)){ // 3D
		if (impl->cur_projection_win != this){
			impl->cur_projection_win = this;
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
		if (impl->whole_window){
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

vector Window::Unproject(const vector &p)
{
	vector r;
	vector pp = p;
	if ((type == ViewPerspective) || (type == ViewIsometric)){ // 3D
		if (impl->cur_projection_win != this){
			impl->cur_projection_win = this;
			NixSetProjectionMatrix(projection);
			NixSetView(mat);
		}
		NixGetVecUnproject(r,pp);
	}else if (type == View2D){
		r.x=(pp.x-MaxX/2)/cam->zoom+cam->pos.x;
		r.y=(pp.y-MaxY/2)/cam->zoom+cam->pos.y;
		r.z=0;
	}else{ // 2D
		if (impl->whole_window){
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

vector Window::GetDirection()
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
		return cam->ang * e_z;
	return v_0;
}

vector Window::GetDirectionUp()
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
		return cam->ang * e_y;
	return v_0;
}

vector Window::GetDirectionRight()
{
	vector d=GetDirection();
	vector u=GetDirectionUp();
	return VecCrossProduct(d,u);
}

void Window::GetMovingFrame(vector &dir, vector &up, vector &right)
{
	dir = GetDirection();
	up = GetDirectionUp();
	right = dir ^ up;
}

};
