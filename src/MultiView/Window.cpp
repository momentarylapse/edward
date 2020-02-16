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
#include "ColorScheme.h"
#include "DrawingHelper.h"
#include "../Edward.h"
#include "../lib/nix/nix.h"



namespace nix{
	extern matrix view_matrix;
	extern matrix projection_matrix;
}

namespace MultiView{

extern nix::Shader *shader_lines_3d;


#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))



Window::Window(MultiView *_view, int _type)
{
	multi_view = _view;
	cam = &multi_view->cam;
	type = _type;
}



#define GRID_CONST	5.0f

float Window::get_grid_d() {
	return exp10(ceil(log10(GRID_CONST / zoom())));
}

int grid_level(int i) {
	if (i == 0)
		return 0;
	if (i % 10 == 0)
		return 1;
	if (i % 100 == 0)
		return 2;
	return 3;
}

const float LOW_MAX = 0.25f;
const float MID_MAX = 0.5f;

float grid_density(int level, float d_err) {
	if (level == 0)
		return 1;
	if (level == 1)
		d_err += 1;
	return min(pow(10.0f, d_err-1.0f) * LOW_MAX, MID_MAX);
}

rect win_get_bounds(Window *w, const vector &ax1, const vector &ax2) {
	vector p[4];
	p[0] = w->unproject(vector(w->dest.x1, w->dest.my(), 0), w->cam->pos);
	p[1] = w->unproject(vector(w->dest.x2, w->dest.my(), 0), w->cam->pos);
	p[2] = w->unproject(vector(w->dest.mx(), w->dest.y1, 0), w->cam->pos);
	p[3] = w->unproject(vector(w->dest.mx(), w->dest.y2, 0), w->cam->pos);

	rect r = rect::ID;
	for (int i=0; i<4; i++) {
		float x = p[i] * ax1;
		float y = p[i] * ax2;
		if (i == 0 or x < r.x1)
			r.x1 = x;
		if (i == 0 or x > r.x2)
			r.x2 = x;
		if (i == 0 or y < r.y1)
			r.y1 = y;
		if (i == 0 or y > r.y2)
			r.y2 = y;
	}
	return r;
}

void draw_grid_3d(const color &bg, Window *w, int plane, float alpha) {

	//msg_write("grid " + f2s(alpha, 3));
	//return exp10(ceil(log10(GRID_CONST / zoom())));
	float D = w->get_grid_d();
	float DERR = log10(D) - log10(GRID_CONST / w->zoom());

	Array<vector> p[4];
	Array<color> col[4];

	vector dir_1, dir_2;
	if (plane == 2) {
		dir_1 = vector::EX;
		dir_2 = vector::EY;
	} else if (plane == 1) {
		dir_1 = vector::EX;
		dir_2 = vector::EZ;
	} else if (plane == 0) {
		dir_1 = vector::EY;
		dir_2 = vector::EZ;
	}


	rect r = win_get_bounds(w, dir_1, dir_2);

	int ix0 = int(ceil(r.x1 / D));
	int ix1 = int(floor(r.x2 / D));
	for (int i=ix0; i<=ix1; i++) {
		int level = grid_level(i);
		float dens = grid_density(level, DERR);
		color c = ColorInterpolate(bg, scheme.GRID, alpha * dens);
		p[level].add(dir_1 * (float)(i*D) + dir_2 * r.y1);
		p[level].add(dir_1 * (float)(i*D) + dir_2 * r.y2);
		col[level].add(c);
		col[level].add(c);
	}


	int iy0 = int(ceil(r.y1 / D));
	int iy1 = int(floor(r.y2 / D));
	for (int i=iy0; i<=iy1; i++) {
		int level = grid_level(i);
		float dens = grid_density(level, DERR);
		color c = ColorInterpolate(bg, scheme.GRID, alpha * dens);
		p[level].add(dir_2 * (float)(i*D) + dir_1 * r.x1);
		p[level].add(dir_2 * (float)(i*D) + dir_1 * r.x2);
		col[level].add(c);
		col[level].add(c);
	}

	nix::SetShader(shader_lines_3d_colored);
	for (int l=3; l>=1; l--)
		nix::DrawLinesColored(p[l], col[l], false);
	set_wide_lines(2.0f);
	nix::DrawLinesColored(p[0], col[0], false);
	set_wide_lines(1.0f);
}


void Window::drawGrid()
{
	if (type == VIEW_2D)
		return;

	nix::SetTexture(NULL);
	nix::SetShader(shader_lines_3d_colored);
	nix::SetZ(false, false);

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

	color bg = getBackgroundColor();

	vector d = getDirection();
	d.x = abs(d.x);
	d.y = abs(d.y);
	d.z = abs(d.z);

	float DMIN = 0.4f;

	if (d.z > DMIN)
		draw_grid_3d(bg, this, 2, (d.z - DMIN) / (1-DMIN));
	if (d.y > DMIN)
		draw_grid_3d(bg, this, 1, (d.y - DMIN) / (1-DMIN));
	if (d.x > DMIN)
		draw_grid_3d(bg, this, 0, (d.x - DMIN) / (1-DMIN));
}

color Window::getBackgroundColor()
{
	if (this == multi_view->active_win)
		return scheme.BACKGROUND_SELECTED;
	return scheme.BACKGROUND;
}

quaternion view_ang(int type, Camera *cam)
{
	quaternion ang;
	if (type == VIEW_FRONT){
		ang = quaternion::rotation_a( vector::EY, -pi);
	}else if (type == VIEW_BACK){
		ang = quaternion::ID;
	}else if (type == VIEW_RIGHT){
		ang = quaternion::rotation_a( vector::EY, -pi/2);
	}else if (type == VIEW_LEFT){
		ang = quaternion::rotation_a( vector::EY, pi/2);
	}else if (type == VIEW_TOP){
		ang = quaternion::rotation_a( vector::EX, pi/2);
	}else if (type == VIEW_BOTTOM){
		ang = quaternion::rotation_a( vector::EX, -pi/2);
	}else if (type == VIEW_PERSPECTIVE){
		ang = cam->ang;
	}else if (type == VIEW_ISOMETRIC){
		ang = cam->ang;
	}else if (type == VIEW_2D){
		ang = quaternion::rotation_a( vector::EY, -pi);
	}
	return ang;
}

string view_name(int type)
{
	if (type == VIEW_FRONT){
		return _("Front");
	}else if (type == VIEW_BACK){
		return _("Back");
	}else if (type == VIEW_RIGHT){
		return _("Right");
	}else if (type == VIEW_LEFT){
		return _("Left");
	}else if (type == VIEW_TOP){
		return _("Top");
	}else if (type == VIEW_BOTTOM){
		return _("Bottom");
	}else if (type == VIEW_PERSPECTIVE){
		return _("Perspective");
	}else if (type == VIEW_ISOMETRIC){
		return _("Isometric");
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
		w->reflection_matrix = matrix::scale( 1, -1, 1);
	}else if (w->type == VIEW_2D){
		float height = w->zoom();
		nix::SetProjectionOrthoExt(cx, cy, -height, height, -1, 1);
		w->reflection_matrix = matrix::scale( -1, 1, 1);
	}else{
		float height = w->zoom();
		nix::SetProjectionOrthoExt(cx, cy, height, -height, - r * 100, r * 100);
		w->reflection_matrix = matrix::scale( 1, -1, 1);
	}
	w->projection_matrix = nix::projection_matrix;

}

void Window::draw()
{
	nix::SetScissor(rect(dest.x1, dest.x2+1, dest.y1, dest.y2));
	nix::SetTexture(NULL);

	color bg = getBackgroundColor();

	set_projection_matrix(this);

	// background color
	nix::ResetToColor(bg);
	nix::SetShader(nix::default_shader_2d);

	// camera matrix
	vector pos = cam->pos;
	string view_kind = view_name(type);
	local_ang = view_ang(type, cam);
	if (type == VIEW_PERSPECTIVE){
		if (!cam->ignore_radius)
			pos -= cam->radius * (cam->ang * vector::EZ);
	}
	nix::SetViewMatrix(matrix::rotation_q(local_ang.bar()) * matrix::translation(-pos));
	view_matrix = nix::view_matrix;
	pv_matrix = projection_matrix * view_matrix;
	ipv_matrix = pv_matrix.inverse();


	nix::SetWorldMatrix(matrix::ID);
	//nix::SetZ(true,true);
	nix::SetZ(type != VIEW_2D, type != VIEW_2D);
	nix::SetWire(false);
	nix::EnableFog(false);
	nix::SetFog(FOG_EXP, 0, 1000, 0, Black); // some shaders need correct fog values
	if (multi_view->grid_enabled)
		drawGrid();

	nix::SetWire(multi_view->wire_mode);
	// light
	vector dir = cam->ang * vector::EZ;
	nix::SetLightDirectional(multi_view->light, dir, White, 0.7f);
	nix::EnableLight(multi_view->light, true);
	nix::SetMaterial(Black,White,Black,0,White);//Black);
	nix::SetColor(White);

	bool index_key = ed->get_key(hui::KEY_I);

	// draw the actual data
	if (ed->cur_mode)
		ed->cur_mode->on_draw_win(this);

	// draw multiview data
	nix::SetShader(nix::default_shader_2d);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetTexture(NULL);
	nix::SetWire(false);
	foreachi(DataSet &d, multi_view->data, di){
		if (d.drawable){
			for (int i=0;i<d.data->num;i++){

				SingleData *sd = MVGetSingleData(d, i);
				if (sd->view_stage < multi_view->view_stage)
					continue;

				vector p = project(sd->pos);
				//if (!dest.inside(p.x,  p.y))
				if ((p.x<dest.x1)or(p.y<dest.y1)or(p.x>dest.x2)or(p.y>dest.y2)or(p.z<=0)or(p.z>=1))
					continue;
				color c = scheme.POINT;
				float radius = scheme.POINT_RADIUS;
				float z = p.z - 0.0001f;//0.1f;
				if (sd->is_selected){
					c = scheme.POINT_SELECTED;
					//z = 0.05f;
					z = 0;
				}
				if (sd->is_special)
					c = scheme.POINT_SPECIAL;
				if ((multi_view->hover.set == di) and (i == multi_view->hover.index)){
					c = scheme.hoverify(c);
					z = 0.0f;
					radius = scheme.POINT_RADIUS_HOVER;
				}
				nix::SetColor(c);
				nix::DrawRect(	p.x-radius,
								p.x+radius,
								p.y-radius,
								p.y+radius,
								z);
			}
		}
		if (d.indexable and index_key){
			nix::SetColor(scheme.TEXT);
			nix::SetAlpha(ALPHA_SOURCE_ALPHA, ALPHA_SOURCE_INV_ALPHA);
			for (int i=0;i<d.data->num;i++){

				SingleData *sd = MVGetSingleData(d, i);
				if (sd->view_stage < multi_view->view_stage)
					continue;
				if (sd->is_selected){
					vector p = project(sd->pos);
					if ((p.x<dest.x1)or(p.y<dest.y1)or(p.x>dest.x2)or(p.y>dest.y2)or(p.z<=0)or(p.z>=1))
						continue;
					nix::DrawStr(p.x+3, p.y, i2s(i));
				}
			}
			nix::SetAlpha(ALPHA_NONE);
		}
	}

	// DEBUG!
	//NixSetZ(false,false);
	//NixDrawLine3D(MouseOverTP,MouseOverTP+vector(0,ViewRadius/3,0),Green);

	// type of view

	// cursor
	if (this != multi_view->mouse_win){
		vector pp = project(multi_view->get_cursor());
		nix::SetShader(nix::default_shader_2d);
		nix::SetColor(scheme.CREATION_LINE);
		nix::DrawRect(pp.x-2, pp.x+2, pp.y-2, pp.y+2, 0);
	}


	if (multi_view->allow_mouse_actions and !multi_view->sel_rect.active)
		if (multi_view->action_con->visible)
			multi_view->action_con->draw(this);

	name_dest = rect(dest.x1 + 3, dest.x1 + 3 + nix::GetStrWidth(view_kind), dest.y1, dest.y1 + 20);

	nix::SetShader(nix::default_shader_2d);
	nix::SetColor(scheme.WINDOW_TITLE);
	bg = scheme.WINDOW_TITLE_BG;
	if (ed->is_active("nix-area") and (this == multi_view->active_win)) {}
		// active?!?
	if ((this == multi_view->mouse_win) and (multi_view->hover.meta == multi_view->hover.HOVER_WINDOW_LABEL))
		bg = scheme.hoverify(bg);
	draw_str_bg(dest.x1 + 3, dest.y1 + 3, view_kind.upper(), scheme.WINDOW_TITLE, bg, TextAlign::LEFT);
	nix::SetColor(scheme.TEXT);

	for (auto &m: multi_view->message3d){
		vector p = project(m.pos);
		if (p.z > 0)
			draw_str(p.x, p.y, m.str);
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
	return local_ang * vector::EZ;
}

vector Window::getDirectionUp()
{
	return local_ang * vector::EY;
}

vector Window::getDirectionRight()
{
	return local_ang * vector::EX;
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
