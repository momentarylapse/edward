//
// Created by michi on 11.05.25.
//

#include "MultiViewWindow.h"
#include "MultiView.h"
#include "MultiViewMeshEmitter.h"
#include "Mode.h"
#include "DrawingHelper.h"
#include <lib/yrenderer/scene/MeshEmitter.h>
#include <lib/yrenderer/scene/SceneRenderer.h>
#include <y/world/components/Camera.h>
#include <Session.h>
#include <lib/math/mat3.h>
#include <lib/image/Painter.h>
#include <lib/math/plane.h>
#include <lib/xhui/Theme.h>
#include <cmath>


MultiViewWindow::MultiViewWindow(MultiView* _multi_view) {
	multi_view = _multi_view;
	scene_renderer = new yrenderer::SceneRenderer(multi_view->session->ctx, yrenderer::RenderPathType::Forward, *multi_view->view_port.scene_view.get());
	scene_renderer->add_emitter(new MultiViewBackgroundEmitter(multi_view));
	scene_renderer->add_emitter(new MultiViewGeometryEmitter(this));
}

vec3 MultiViewWindow::view_pos() const {
	return multi_view->view_port.pos - local_ang * vec3(0, 0, multi_view->view_port.radius);
}

quaternion MultiViewWindow::view_ang() const {
	return local_ang;
}



vec3 MultiViewWindow::project(const vec3& v) const {
	return to_pixels.project(v);
}

vec3 MultiViewWindow::unproject(const vec2& v, const vec3& zref) const {
	vec3 op = project(zref);
	vec3 r = vec3(v, op.z);
	return to_pixels.inverse().project(r);
}



vec3 linearized_projection1(const mat4& m, const vec3& p, const vec3& E) {
	float w = (p.x*m._30 + p.y*m._31 + p.z*m._32 + m._33);
	vec3 m3 = {m._30, m._31, m._32};
	return m.transform_normal(E) / w - (m*p) * vec3::dot(m3, E) / (w*w);
}

vec3 linearized_projection2(const mat4& m, const vec3& p, const vec3& dir) {
	float epsilon = 0.001;
	return (m.project(p + epsilon * dir) - m.project(p)) / epsilon;
}

mat3 linearized_projection(const mat4& m, const vec3& p) {
	// d/dx m.project(p + x*E)
	// d/dx m * (p + x*E) / (<(_30,_31,_32), p+x*E> + _33);
	// d/dx (m*p + x*m_0*E) / (<(_30,_31,_32), p> + _33 + x*<(_30,_31,_32),E>);
	// m_0*E / (<_3,p>+_33) - (m*p)*<_3,E>/ (<_3,p>+_33)^2

	// d/dx  (a+xb) / (c+xd)
	// d/dx f/g = f'/g - fg'/gg
	// b/c - ad/cc

	return mat3(linearized_projection1(m, p, {1,0,0}), linearized_projection1(m, p, {0,1,0}), linearized_projection1(m, p, {0,0,1}));

	float w = (p.x*m._30 + p.y*m._31 + p.z*m._32 + m._33);
	mat3 r;
	for (int i=0; i<3; i++)
		for (int k=0; k<3; k++)
			r.__e[i][k] = m.__e[i][k] / w;
	for (int i=0; i<3; i++) {
		const auto pp = (m*p) * r.e[i*4+3] / (w*w);
		if (false) {
			r.__e[i][0] -= pp.x;
			r.__e[i][1] -= pp.y;
			r.__e[i][2] -= pp.z;
		} else {
			r.__e[0][i] -= pp.x;
			r.__e[1][i] -= pp.y;
			r.__e[2][i] -= pp.z;
		}
	}

	return r;
}

mat3 MultiViewWindow::linear_projection(const vec3& p) const {
	return linearized_projection(to_pixels, p);
}


base::optional<vec3> MultiViewWindow::grid_hover_point(const vec2& m) const {
	const auto d = active_grid_direction();
	vec3 p0 = to_pixels.inverse().project({m, 0});
	vec3 p1 = to_pixels.inverse().project({m, 1});

	plane grid_plane = plane::from_point_normal({0,0,0}, d);
	vec3 p;
	grid_plane.intersect_line(p0, p1, p);
	// the camera is on the "wrong" side of the grid?
	if (vec3::dot(p0 - p, p0 - p1) < 0)
		return base::None;
	return p;
}

vec3 MultiViewWindow::direction() const {
	return multi_view->view_port.ang * vec3::EZ;
}

float MultiViewWindow::zoom() const {

	//return 1000.0f / radius;
	//if (multi_view->mode3d)
		return area.height() / multi_view->view_port.radius;
	//else
	//	return area.height() * 0.8f / multi_view->view_port.radius;
}

float MultiViewWindow::pixel_to_size(float s) const {
	return s / zoom();
}


#define GRID_CONST	5.0f

float MultiViewWindow::get_grid_d() const {
	return pow(10.0f, ceil(log10(GRID_CONST / zoom())));
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
	return min((float)pow(10.0f, d_err-1.0f) * LOW_MAX, MID_MAX);
}


int MultiViewWindow::active_grid() const {
	vec3 d = direction();
	d.x = abs(d.x);
	d.y = abs(d.y);
	d.z = abs(d.z);
	if (d.x > d.y and d.x > d.z)
		return 0;
	if (d.y > d.z)
		return 1;
	return 2;
}

vec3 MultiViewWindow::active_grid_direction() const {
	vec3 dd = vec3::EZ;
	int ag = active_grid();
	if (ag == 0)
		dd = vec3::EX;
	if (ag == 1)
		dd = vec3::EY;

	if (dd * direction() < 0)
		return -dd;
	return dd;
}

mat3 MultiViewWindow::active_grid_frame() const {
	const vec3 dir = active_grid_direction();
	const vec3 up = dir.ortho();
	const vec3 right = dir ^ up;
	return {right, up, dir};
}

mat3 MultiViewWindow::edit_frame() const {
	return active_grid_frame();
}

void MultiViewWindow::prepare(const yrenderer::RenderParams& params) {
	projection = multi_view->view_port.cam.projection_matrix(area.width() / area.height());

	auto cam = multi_view->view_port.cam;
	cam.pos = view_pos();
	cam.ang = view_ang();
	scene_renderer->set_view(params, cam, &projection);
	view = scene_renderer->rvd.ubo.v;
	scene_renderer->prepare(params);

	// 3d -> pixel
	to_pixels = mat4::translation({area.x1, area.y1, 0})
		* mat4::scale(area.width()/2, area.height()/2, 1)
		* mat4::translation({1.0f, 1.0f, 0})
		* projection * view;
}



void add_grid(const rect &r, Array<vec3> p[4], Array<color> col[4], float D, float DERR, const vec3 &dir_1, const vec3 &dir_2, float alpha) {

	int ix0 = int(ceil(r.x1 / D));
	int ix1 = int(floor(r.x2 / D));
	for (int i=ix0; i<=ix1; i++) {
		int level = grid_level(i);
		float dens = grid_density(level, DERR);
		color c = xhui::Theme::_default.background_hover.srgb_to_linear(); //ColorInterpolate(bg, scheme.GRID, alpha * dens);
		c.a = alpha * dens;
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
		color c = xhui::Theme::_default.background_hover.srgb_to_linear();//scheme.GRID;//ColorInterpolate(bg, scheme.GRID, alpha * dens);
		c.a = alpha * dens;
		p[level].add(dir_2 * (float)(i*D) + dir_1 * r.x1);
		p[level].add(dir_2 * (float)(i*D) + dir_1 * r.x2);
		col[level].add(c);
		col[level].add(c);
	}
}

rect win_get_bounds(MultiViewWindow *w, const vec3 &ax1, const vec3 &ax2) {
	vec3 p[4];
	rect dest = w->area_native;
	vec3 pos0 = w->multi_view->view_port.pos;
	p[0] = w->unproject(vec2(dest.x1, dest.center().y), pos0);
	p[1] = w->unproject(vec2(dest.x2, dest.center().y), pos0);
	p[2] = w->unproject(vec2(dest.center().x, dest.y1), pos0);
	p[3] = w->unproject(vec2(dest.center().x, dest.y2), pos0);
	p[0] = w->unproject(dest.p00(), pos0);
	p[1] = w->unproject(dest.p10(), pos0);
	p[2] = w->unproject(dest.p01(), pos0);
	p[3] = w->unproject(dest.p11(), pos0);

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

void draw_grid_3d(const color &bg, MultiViewWindow *w, int plane, float alpha) {

	//msg_write("grid " + f2s(alpha, 3));
	//return exp10(ceil(log10(GRID_CONST / zoom())));
	float D = w->get_grid_d();
	float DERR = log10(D) - log10(GRID_CONST / w->zoom());

	Array<vec3> p[4];
	Array<color> col[4];

	vec3 dir_1, dir_2;
	if (plane == 2) {
		dir_1 = vec3::EX;
		dir_2 = vec3::EY;
	} else if (plane == 1) {
		dir_1 = vec3::EX;
		dir_2 = vec3::EZ;
	} else if (plane == 0) {
		dir_1 = vec3::EY;
		dir_2 = vec3::EZ;
	}


	rect r = win_get_bounds(w, dir_1, dir_2);
	add_grid(r, p, col, D, DERR, dir_1, dir_2, alpha);
	/*add_grid(move_rect(r, 1, 0), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);
	add_grid(move_rect(r, -1, 0), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);
	add_grid(move_rect(r, 1, -1), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);
	add_grid(move_rect(r, 0, -1), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);
	add_grid(move_rect(r, -1, -1), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);
	add_grid(move_rect(r, 1, 1), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);
	add_grid(move_rect(r, 0, 1), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);
	add_grid(move_rect(r, -1, 1), p, col, D, DERR, dir_1, dir_2, alpha*0.5f);*/


	auto dh = w->multi_view->session->drawing_helper;
	dh->set_blending(true);
	dh->set_line_width(2);//scheme.LINE_WIDTH_THIN);
	for (int l=3; l>=1; l--)
		dh->draw_lines_colored(p[l], col[l], false);
	dh->set_line_width(3);//scheme.LINE_WIDTH_MEDIUM);
	dh->draw_lines_colored(p[0], col[0], false);
	dh->set_blending(false);
}

void MultiViewWindow::draw(const yrenderer::RenderParams& params) {
	multi_view->session->drawing_helper->set_window(this);

	scene_renderer->draw(params);

	if (!multi_view->_show_grid)
		return;

	color bg = xhui::Theme::_default.background;//get_background_color();

	vec3 d = direction();
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

void MultiViewWindow::draw_post(Painter* p) {
	if (multi_view->hover and multi_view->_show_grid and !multi_view->selection_area) {
		const auto p0 = multi_view->hover->tp;
		const float r = 1.5f;
		auto px = project({p0.x, 0, 0});
		p->set_color(color(1, 1, 0.2f, 0.2f));
		if (px.z > 0 and px.z < 1)
			p->draw_circle(px.xy(), r);
		auto py = project({0, p0.y, 0});
		p->set_color(color(1, 0.2f, 1, 0.2f));
		if (py.z > 0 and py.z < 1)
			p->draw_circle(py.xy(), r);
		auto pz = project({0, 0, p0.z});
		p->set_color(color(1, 0.2f, 0.2f, 1));
		if (px.z > 0 and pz.z < 1)
			p->draw_circle(pz.xy(), r);
	}
}


yrenderer::RenderViewData& MultiViewWindow::rvd() {
	return scene_renderer->rvd;
}


