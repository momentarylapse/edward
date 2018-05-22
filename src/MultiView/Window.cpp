/*
 * Window.cpp
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#include "Window.h"
#include "MultiView.h"
#include "ActionController.h"
#include "SingleData.h"
#include "../Edward.h"


namespace nix{
	extern matrix view_matrix;
	extern matrix projection_matrix;
}

namespace MultiView{

extern nix::Shader *shader_lines_3d;

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

Window::Window(MultiView *_view, int _type)
{
	multi_view = _view;
	cam = &multi_view->cam;
	type = _type;
}



#define GRID_CONST	5.0f

float Window::get_grid_d()
{
	return exp10(ceil(log10(GRID_CONST / zoom())));
}

void Window::drawGrid()
{
	if (type == VIEW_ISOMETRIC)
		return;
	rect d;
	vector bg_a,bg_b;
	Array<vector> p;

	nix::SetTexture(NULL);

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

	if (type == VIEW_2D)
		return;
	color bg = getBackgroundColor();

	// spherical for perspective view
	if (type == VIEW_PERSPECTIVE){
		nix::SetShader(shader_lines_3d);
		vector PerspectiveViewPos = cam->radius * (cam->ang * e_z) - cam->pos;
		//NixSetZ(false,false);
		// horizontal
		float r = cam->radius * 1000 * 0.6f;
		for (int j=-16;j<16;j++)
			for (int i=0;i<64;i++){
				if (j == 0)
					continue;
				vector pa = vector(float(j)/32*pi,float(i  )/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				vector pb = vector(float(j)/32*pi,float(i+1)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				p.add(pa);
				p.add(pb);
			}
		// vertical
		for (int j=0;j<32;j++){
			if (j == 0)
				continue;
			for (int i=0;i<64;i++){
				vector pa = vector(float(i  )/32*pi,float(j)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				vector pb = vector(float(i+1)/32*pi,float(j)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
				p.add(pa);
				p.add(pb);
			}
		}
		nix::SetColor(ColorInterpolate(bg, multi_view->ColorGrid, 0.1f));
		nix::DrawLines(p, false);

		p.clear();
		for (int i=0;i<64;i++){
			vector pa = vector(0,float(i  )/32*pi,0).ang2dir() * r - PerspectiveViewPos;
			vector pb = vector(0,float(i+1)/32*pi,0).ang2dir() * r - PerspectiveViewPos;
			p.add(pa);
			p.add(pb);
		}
		// vertical
		for (int i=0;i<64;i++){
			vector pa = vector(float(i  )/32*pi,0,0).ang2dir() * r - PerspectiveViewPos;
			vector pb = vector(float(i+1)/32*pi,0,0).ang2dir() * r - PerspectiveViewPos;
			p.add(pa);
			p.add(pb);
		}
		nix::SetColor(ColorInterpolate(bg, multi_view->ColorGrid, 0.6f));
		nix::DrawLines(p, false);
		//NixSetZ(true,true);
		return;
	}

	// rectangular
	float D = get_grid_d();
	int a,b;
	float fa,fb,t;

	vector vux1 = unproject(vector(dest.x1,0,0));
	vector vux2 = unproject(vector(dest.x2,0,0));
	vector vuy1 = unproject(vector(0,dest.y1,0));
	vector vuy2 = unproject(vector(0,dest.y2,0));
	vector n,va,vb;
	nix::SetShader(nix::default_shader_2d);

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
		int x=(int)project(vector((float)i*D,(float)i*D,(float)i*D)).x;
		nix::SetColor(ColorInterpolate(bg, multi_view->ColorGrid, GetDensity(i,(float)nix::target_width/(fb-fa))));
		nix::DrawLineV(x,dest.y1,dest.y2,0.99998f-GetDensity(i,(float)nix::target_width/(fb-fa))*0.00005f);
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
		int y=(int)project(vector((float)i*D,(float)i*D,(float)i*D)).y;
		nix::SetColor(ColorInterpolate(bg, multi_view->ColorGrid, GetDensity(i,(float)nix::target_width/(fb-fa))));
		nix::DrawLineH(dest.x1,dest.x2,y,0.99998f-GetDensity(i,(float)nix::target_width/(fb-fa))*0.00005f);
	}
}

color Window::getBackgroundColor()
{
	if (this == multi_view->active_win)
		return multi_view->ColorBackGroundSelected;
	return multi_view->ColorBackGround;
}

quaternion view_ang(int type, Camera *cam)
{
	quaternion ang;
	if (type == VIEW_FRONT){
		QuaternionRotationA(ang, e_y, -pi);
	}else if (type == VIEW_BACK){
		ang = q_id;
	}else if (type == VIEW_RIGHT){
		QuaternionRotationA(ang, e_y, -pi/2);
	}else if (type == VIEW_LEFT){
		QuaternionRotationA(ang, e_y, pi/2);
	}else if (type == VIEW_TOP){
		QuaternionRotationA(ang, e_x, pi/2);
	}else if (type == VIEW_BOTTOM){
		QuaternionRotationA(ang, e_x, -pi/2);
	}else if (type == VIEW_PERSPECTIVE){
		ang = cam->ang;
	}else if (type == VIEW_ISOMETRIC){
		ang = cam->ang;
	}else if (type == VIEW_2D){
		QuaternionRotationA(ang, e_y, -pi);
	}
	return ang;
}

string view_name(int type)
{
	if (type == VIEW_FRONT){
		return _("Vorne");
	}else if (type == VIEW_BACK){
		return _("Hinten");
	}else if (type == VIEW_RIGHT){
		return _("Rechts");
	}else if (type == VIEW_LEFT){
		return _("Links");
	}else if (type == VIEW_TOP){
		return _("Oben");
	}else if (type == VIEW_BOTTOM){
		return _("Unten");
	}else if (type == VIEW_PERSPECTIVE){
		return _("Perspektive");
	}else if (type == VIEW_ISOMETRIC){
		return _("Isometrisch");
	}else if (type == VIEW_2D){
		return _("2D");
	}
	return "???";
}

void set_projection_matrix(Window *w)
{
	float r = w->cam->radius;
	float cx = (w->dest.x1 + w->dest.x2) / 2;
	float cy = (w->dest.y1 + w->dest.y2) / 2;
	if (w->type == VIEW_PERSPECTIVE){
		float height = w->dest.height();
		nix::SetProjectionPerspectiveExt(cx, cy, height, height, r / 1000, r * 1000);
		MatrixScale(w->reflection_matrix, 1, -1, 1);
	}else if (w->type == VIEW_2D){
		float height = w->zoom();
		nix::SetProjectionOrthoExt(cx, cy, -height, height, -1, 1);
		MatrixScale(w->reflection_matrix, -1, 1, 1);
	}else{
		float height = w->zoom();
		nix::SetProjectionOrthoExt(cx, cy, height, -height, - r * 100, r * 100);
		MatrixScale(w->reflection_matrix, 1, -1, 1);
	}
	w->projection_matrix = nix::projection_matrix;

}

void Window::draw()
{
	matrix rot, trans;
	nix::Scissor(dest);
	string view_kind;
	nix::EnableLighting(false);
	nix::SetTexture(NULL);

	color bg = getBackgroundColor();

	set_projection_matrix(this);

	// background color
	nix::ResetToColor(bg);
	nix::SetShader(nix::default_shader_2d);

	// camera matrix
	vector pos = cam->pos;
	view_kind = view_name(type);
	local_ang = view_ang(type, cam);
	if (type == VIEW_PERSPECTIVE){
		if (!cam->ignore_radius)
			pos -= cam->radius * (cam->ang * e_z);
	}
	nix::SetViewPosAng(pos, local_ang);
	view_matrix = nix::view_matrix;
	pv_matrix = projection_matrix * view_matrix;
	MatrixInverse(ipv_matrix, pv_matrix);


	nix::SetWorldMatrix(m_id);
	//nix::SetZ(true,true);
	nix::SetZ(type != VIEW_2D, type != VIEW_2D);
	nix::SetWire(false);
	nix::EnableLighting(false);
	nix::EnableFog(false);
	nix::SetFog(FOG_EXP, 0, 1000, 0, Black); // some shaders need correct fog values
	if (multi_view->grid_enabled)
		drawGrid();

	nix::SetWire(multi_view->wire_mode);
	// light
	vector dir = cam->ang * e_z;
	nix::SetLightDirectional(multi_view->light, dir, color(1,0.6f,0.6f,0.6f), 0.5f, 0.7f);
	nix::EnableLight(multi_view->light, true);
	nix::EnableLighting(multi_view->light_enabled);
	nix::SetAmbientLight(Black);
	nix::SetMaterial(Black,White,Black,0,White);//Black);
	nix::SetColor(White);

	bool index_key = ed->getKey(hui::KEY_I);

	// draw the actual data
	if (ed->cur_mode)
		ed->cur_mode->onDrawWin(this);

	// draw multiview data
	nix::SetShader(nix::default_shader_2d);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetTexture(NULL);
	nix::SetWire(false);
	nix::EnableLighting(false);
	foreachi(DataSet &d, multi_view->data, di){
		if (d.drawable or d.indexable){
			for (int i=0;i<d.data->num;i++){

				SingleData *sd = MVGetSingleData(d, i);
				if (sd->view_stage < multi_view->view_stage)
					continue;

				bool _di = (d.indexable and sd->is_selected and index_key);
				if (!d.drawable and !_di)
					continue;
				vector p = project(sd->pos);
				//if (!dest.inside(p.x,  p.y))
				if ((p.x<dest.x1)or(p.y<dest.y1)or(p.x>dest.x2)or(p.y>dest.y2)or(p.z<=0)or(p.z>=1))
					continue;
				if (_di)
					nix::DrawStr(p.x+3, p.y, i2s(i));
				if (d.drawable){
					color c = multi_view->ColorPoint;
					float radius = (float)multi_view->POINT_RADIUS;
					float z = p.z - 0.0001f;//0.1f;
					if (sd->is_selected){
						c = multi_view->ColorPointSelected;
						//z = 0.05f;
						z = 0;
					}
					if (sd->is_special)
						c = multi_view->ColorPointSpecial;
					if ((multi_view->hover.set == di) and (i == multi_view->hover.index)){
						c = color(c.a,c.r+0.4f,c.g+0.4f,c.b+0.4f);
						z = 0.0f;
						radius = (float)multi_view->POINT_RADIUS_HOVER;
					}
					nix::SetColor(c);
					nix::DrawRect(	p.x-radius,
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

	if (multi_view->action_con->visible)
		multi_view->action_con->draw(this);

	name_dest = rect(dest.x1 + 3, dest.x1 + 3 + nix::GetStrWidth(view_kind), dest.y1, dest.y1 + 20);

	nix::SetShader(nix::default_shader_2d);
	nix::SetColor(multi_view->ColorWindowType);
	if (ed->isActive("nix-area") and (this == multi_view->active_win))
		nix::SetColor(multi_view->ColorText);
	if ((this == multi_view->mouse_win) and (multi_view->hover.meta == multi_view->hover.HOVER_WINDOW_LABEL))
		nix::SetColor(Red);
	ed->drawStr(dest.x1 + 3, dest.y1, view_kind);
	nix::SetColor(multi_view->ColorText);

	for (auto &m: multi_view->message3d){
		vector p = project(m.pos);
		if (p.z > 0)
			ed->drawStr(p.x, p.y, m.str);
	}
}


vector Window::project(const vector &p)
{
	vector r = pv_matrix.project(p);
	r.x = nix::target_width * (r.x + 1) / 2;
	r.y = nix::target_height * (-r.y + 1) / 2;
	r.z = (r.z + 1) / 2;
	return r;
}

vector Window::unproject(const vector &p)
{
	vector r;
	r.x = p.x*2/nix::target_width - 1;
	r.y = - p.y*2/nix::target_height + 1;
	r.z = p.z*2 - 1;
	return ipv_matrix.project(r);
	//return pv_matrix.unproject(r);
}

vector Window::unproject(const vector &p, const vector &o)
{
	vector op = project(o);
	vector r;
	r.x = p.x*2/nix::target_width - 1;
	r.y = - p.y*2/nix::target_height + 1;
	r.z = op.z*2 - 1;
	return ipv_matrix.project(r);
}

vector Window::getDirection()
{
	return local_ang * e_z;
}

vector Window::getDirectionUp()
{
	return local_ang * e_y;
}

vector Window::getDirectionRight()
{
	return local_ang * e_x;
}

void Window::getMovingFrame(vector &dir, vector &up, vector &right)
{
	dir = getDirection();
	up = getDirectionUp();
	right = dir ^ up;
}

float Window::zoom()
{
	//return 1000.0f / radius;
	if (multi_view->mode3d)
		return dest.height() / cam->radius;
	else
		return dest.height() * 0.8f / cam->radius;
}


};
