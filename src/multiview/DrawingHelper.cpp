/*
 * DrawingHelper.cpp
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#include "DrawingHelper.h"
#include "../lib/nix/nix.h"
#include "../lib/image/image.h"
#include "../lib/os/msg.h"
#include "../lib/math/vec2.h"
#include <y/helper/ResourceManager.h>
#if HAS_LIB_GL
#include "MultiView.h"
#include "Window.h"
#endif
#include "ColorScheme.h"
#if HAS_LIB_GL
#include <cairo/cairo.h>
#endif
#include <pango/pango.h>
#include <pango/pangocairo.h>


namespace MultiView {
	shared<CubeMap> cube_map;
}



Texture *create_round_texture(int n) {
	auto t = new Texture();
	Image im;
	im.create(n, n, color(0,0,0,0));
	for (int i=0; i<n; i++)
		for (int j=0; j<n; j++) {
			float r = sqrt(pow(i - n/2, 2) + pow(j - n/2, 2));
			float f = clamp((n*0.45f - r)*0.5f, 0.0f, 1.0f);
			im.set_pixel(i, j, color(f, 1, 1, 1));
		}
	t->write(im);
	return t;
}

void create_fake_dynamic_cube_map(CubeMap *cube_map) {
	Image im;
	int size = cube_map->width;
	im.create(size, size, White);
	for (int i=0; i<size; i++)
		for (int j=0; j<size; j++) {
			float f = 0;
			if ((i % 32) == 0 or (j % 32) == 0)
				f = 0.125;
			if ((i % 128) == 0 or (j % 128) == 0)
				f = 0.25;
			im.set_pixel(i, j, color::interpolate(scheme.BACKGROUND, scheme.GRID, f));
		}
	for (int i=0;i<6;i++)
		cube_map->write_side(i, im);
}

DrawingHelper::DrawingHelper(Context *_ctx, ResourceManager *rm, const Path &dir) {
	ctx = _ctx;
	resource_manager = rm;
	vb_lines = new VertexBuffer("3f,4f");
	vb_2d = new VertexBuffer("3f,4f,2f");
	tex_round = create_round_texture(32);
	tex_text = new Texture();
	tex_white = new Texture();
	Image im;
	im.create(32, 32, White);
	tex_white->write(im);

	try {

		resource_manager->default_shader = "default.shader";

		resource_manager->load_shader_module(dir | "shader/module-basic-interface.shader");
		resource_manager->load_shader_module(dir | "shader/module-basic-data.shader");
		resource_manager->load_shader_module(dir | "shader/module-lighting-pbr.shader");
		resource_manager->load_shader_module(dir | "shader/forward/module-surface.shader");

		resource_manager->load_shader_module(dir | "shader/module-vertex-default.shader");
		resource_manager->load_shader_module(dir | "shader/module-vertex-animated.shader");

#if HAS_LIB_GL
		shader_lines_3d = ctx->load_shader(dir | "shader/lines-3d.shader");
		shader_lines_3d_colored = ctx->load_shader(dir | "shader/lines-3d-colored.shader");
		shader_lines_3d_colored_wide = ctx->load_shader(dir | "shader/lines-3d-colored-wide.shader");
		shader_selection = resource_manager->load_shader(dir | "shader/selection.shader");
		shader_selection->set_int("num_lights", 1);
#endif
	} catch (Exception &e) {
		msg_error(e.message());
		throw;
	}

	MultiView::cube_map = new CubeMap(256, "rgba:i8");
	create_fake_dynamic_cube_map(MultiView::cube_map.get());
}

void DrawingHelper::set_line_width(float width) {
#if HAS_LIB_GL
	if (width == 1.0f) {
		nix::set_shader(shader_lines_3d_colored.get());
	} else {
		auto s = shader_lines_3d_colored_wide.get();
		nix::set_shader(s);
		s->set_float("target_width", (float)nix::target_width);
		s->set_float("target_height", (float)nix::target_height);
		s->set_float("line_width", width);
	}
#endif
}

void DrawingHelper::set_color(const color &c) {
	_cur_color_ = c;
}



void DrawingHelper::draw_line_2d(float x1, float y1, float x2, float y2, float depth) {
	draw_line(vec3(x1, y1, depth), vec3(x2, y2, depth));
}

void DrawingHelper::draw_lines(const Array<vec3> &p, bool contiguous) {
	Array<color> c;
	c.resize(p.num);
	for (int i=0; i<c.num; i++)
		c[i] = _cur_color_;
	draw_lines_colored(p, c, contiguous);
}

struct LineVertex {
	vec3 p;
	color c;
};

void DrawingHelper::draw_lines_colored(const Array<vec3> &p, const Array<color> &c, bool contiguous) {
	//set_line_width(2);
	Array<LineVertex> v;
	for (int i=0; i<p.num; i++)
		v.add({p[i], c[i]});
	vb_lines->update(v);
#if HAS_LIB_GL
	nix::draw_lines(vb_lines, contiguous);
#endif
}

void DrawingHelper::draw_line(const vec3 &l1, const vec3 &l2) {
	draw_lines_colored({l1, l2}, {_cur_color_, _cur_color_}, false);
}


void DrawingHelper::draw_circle(const vec3 &pos, const vec3 &n, float radius) {

	vec3 e1 = n.ortho();
	vec3 e2 = n ^ e1;
	e1 *= radius;
	e2 *= radius;
	int N = 64;
	Array<vec3> p;
	for (int i=0; i<=N; i++) {
		float w = i * 2 * pi / N;
		p.add(pos + sin(w) * e1 + cos(w) * e2);
	}
	draw_lines(p, true);
}


void DrawingHelper::draw_helper_line(MultiView::Window *win, const vec3 &a, const vec3 &b) {
#if HAS_LIB_GL
	nix::set_z(false, false);
	set_color(scheme.TEXT);
	set_line_width(scheme.LINE_WIDTH_HELPER);
	draw_line(a, b);
	//nix::set_z(true, true);
	vec3 pa = win->project(a);
	vec3 pb = win->project(b);
	//vector d = (pb - pa).normalized();
	//vector e = d ^ vector::EZ;
	float r = 3;
	nix::set_shader(win->ctx->default_2d.get());
	draw_rect(pa.x-r, pa.x+r, pa.y-r, pa.y+r, 0);
	draw_rect(pb.x-r, pb.x+r, pb.y-r, pb.y+r, 0);
#endif
}


void DrawingHelper::draw_rect(float x1, float x2, float y1, float y2, float depth) {
	draw_2d(rect::ID, rect(x1, x2, y1, y2), depth);
}

struct Vertex2d {
	vec3 p;
	color c;
	vec2 uv;
};

void DrawingHelper::draw_2d(const rect &src, const rect &dest, float depth) {
#if HAS_LIB_GL
	vec3 a = vec3(dest.x1, dest.y1, depth);
	vec3 b = vec3(dest.x2, dest.y1, depth);
	vec3 c = vec3(dest.x1, dest.y2, depth);
	vec3 d = vec3(dest.x2, dest.y2, depth);
	color col = _cur_color_;

	vb_2d->update(Array<Vertex2d>{{a, col, {src.x1,src.y1}},{b, col, {src.x2,src.y1}},{c, col, {src.x1,src.y2}},
	                              {c, col, {src.x1,src.y2}},{b, col, {src.x2,src.y1}},{d, col, {src.x2,src.y2}}});
	nix::draw_triangles(vb_2d);
#endif
}



void DrawingHelper::draw_round_rect(const rect &r) {

	float R = scheme.BOX_ROUNDNESS;
	float x[4] = {r.x1, r.x1 + R, r.x2 - R, r.x2};
	float y[4] = {r.y1, r.y1 + R, r.y2 - R, r.y2};
	float u[4] = {0, 0.5f, 0.5f, 1};
	/*vector p[16], n[16];
	float uv[32];

	nix::vb_temp->clear();
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++) {
		}
	nix::vb_temp->addTrias(18, )*/

	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
			draw_2d(rect(u[i], u[i+1], u[j], u[j+1]),    rect(x[i], x[i+1], y[j], y[j+1]), 0);
}


int DrawingHelper::get_str_width(const string &str) {
	Image im;
	render_text(str, im);
	return im.width;
}

void DrawingHelper::_draw_str(float x, float y, const string &str) {
	Image im;
	render_text(str, im);
#if HAS_LIB_GL
	if (im.width > 0) {
		tex_text->write(im);
		nix::bind_texture(0, tex_text);
		draw_2d(rect::ID, rect(x, x + im.width, y, y + im.height), 0);
	}
#endif
}

void DrawingHelper::draw_str_bg(int x, int y, const string &str, const color &fg, const color &bg, TextAlign align) {
#if HAS_LIB_GL
	color c0 = _cur_color_;
	auto xx = str.explode("\n");
	float line_h = scheme.TEXT_LINE_HEIGHT;
	float h = line_h * xx.num;
	Array<int> ww;
	int wmax = 0;
	for (string &s: xx) {
		int w = get_str_width(s);
		ww.add(w);
		wmax = max(wmax, w);
	}
	if (align == TextAlign::RIGHT)
		x -= wmax;
	else if (align == TextAlign::CENTER)
		x -= wmax / 2;
	nix::bind_texture(0, tex_round);
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	float r = scheme.BOX_PADDING;
	set_color(bg);
	draw_round_rect(rect(float(x-r), float(x+wmax+r), float(y-r), float(y+h+r)));
	set_color(fg);
	nix::bind_texture(0, nullptr);
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	foreachi (string &s, xx, i) {
		if (align == TextAlign::RIGHT)
			_draw_str(x+wmax-ww[i], y+line_h*i, s);
		else if (align == TextAlign::CENTER)
			_draw_str(x+wmax/2-ww[i]/2, y+line_h*i, s);
		else if (align == TextAlign::LEFT)
			_draw_str(x, y+line_h*i, s);
	}
	nix::disable_alpha();
	set_color(c0);
#endif
}

void DrawingHelper::draw_str(int x, int y, const string &str, TextAlign a) {
	color c0 = _cur_color_;
	draw_str_bg(x, y, str, c0, scheme.TEXT_BG, a);
}







void DrawingHelper::set_font_size(float size) {
	font_size = size;
}

void DrawingHelper::set_font(const string &name, float size) {
	font_name = name;
	font_size = size;
}


void DrawingHelper::render_text(const string &text, Image &im) {
	if (text.num == 0) {
		im.clear();
		return;
	}
	bool failed = false;
	cairo_surface_t *surface;
	cairo_t *cr;

	// initial surface size guess
	int w_surf = 1024;
	int h_surf = font_size * 2;

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w_surf, h_surf);
	cr = cairo_create(surface);

	cairo_set_source_rgba(cr, 0, 0, 0, 1);
	cairo_rectangle(cr, 0, 0, w_surf, h_surf);
	cairo_fill(cr);

	int x = 0, y = 0;

	cairo_set_source_rgba(cr, 1, 1, 1, 1);

	PangoLayout *layout = pango_cairo_create_layout(cr);
	PangoFontDescription *desc = pango_font_description_from_string((font_name + " " + f2s(font_size, 1)).c_str());
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	pango_layout_set_text(layout, (char*)text.data, text.num);
	//int baseline = pango_layout_get_baseline(layout) / PANGO_SCALE;
	int w_used, h_used;
	pango_layout_get_pixel_size(layout, &w_used, &h_used);

	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);

	cairo_surface_flush(surface);
	unsigned char *c0 = cairo_image_surface_get_data(surface);
	im.create(w_used, h_used, White);
	for (int y=0;y<h_used;y++) {
		unsigned char *c = c0 + 4 * y * w_surf;
		for (int x=0;x<w_used;x++) {
			float a = (float)c[1] / 255.0f;
			im.set_pixel(x, y, color(a, a, 1, 1));
			c += 4;
		}
	}
	im.alpha_used = true;

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}


void DrawingHelper::set_material_selected() {
#if HAS_LIB_GL
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_shader(shader_selection.get());
	nix::set_material(color(0.3f,0,0,0), 0, 0, color(1, 0.8f,0,0));
	//nix::set_material(Black,color(0.3f,0,0,0),Black,0,color(1, 0.5f, 0, 1));
	nix::bind_texture(0, nullptr);
#endif
}

void DrawingHelper::set_material_hover() {
#if HAS_LIB_GL
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_shader(shader_selection.get());
	nix::set_material(color(0.5f,0,0,0), 0, 0, White);
	nix::bind_texture(0, nullptr);
#endif
}

void DrawingHelper::set_material_creation(float intensity) {
#if HAS_LIB_GL
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_shader(shader_selection.get());
	nix::set_material(color(0.3f*intensity,0.3f,1,0.3f), 0, 1, color(1,0.1f,0.4f,0.1f));
	nix::bind_texture(0, nullptr);
#endif
}



