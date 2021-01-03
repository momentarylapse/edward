/*
 * DrawingHelper.cpp
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#include "DrawingHelper.h"
#include "../lib/nix/nix.h"
#include "MultiView.h"
#include "Window.h"
#include "ColorScheme.h"


shared<nix::Shader> shader_selection;
shared<nix::Shader> shader_lines_3d;
shared<nix::Shader> shader_lines_3d_colored;
shared<nix::Shader> shader_lines_3d_colored_wide;

namespace MultiView {
	shared<nix::CubeMap> cube_map;
}

static nix::Texture *tex_round;
static nix::Texture *tex_text;
static string font_name = "Sans";
static float font_size = 12;
static color _cur_color_ = White;

static nix::VertexBuffer *vb_lines = nullptr;
static nix::VertexBuffer *vb_2d = nullptr;


nix::Texture *create_round_texture(int n) {
	auto t = new nix::Texture();
	Image im;
	im.create(n, n, color(0,0,0,0));
	for (int i=0; i<n; i++)
		for (int j=0; j<n; j++) {
			float r = sqrt(pow(i - n/2, 2) + pow(j - n/2, 2));
			float f = clamp((n*0.45f - r)*0.5f, 0.0f, 1.0f);
			im.set_pixel(i, j, color(f, 1, 1, 1));
		}
	t->overwrite(im);
	return t;
}

void create_fake_dynamic_cube_map(nix::CubeMap *cube_map) {
	Image im;
	int size = cube_map->width;
	im.create(size, size, White);
	for (int i=0; i<size; i++)
		for (int j=0; j<size; j++) {
			float f = 0;
			if ((i % 16) == 0 or (j % 16) == 0)
				f = 0.5;
			if ((i % 64) == 0 or (j % 64) == 0)
				f = 1;
			im.set_pixel(i, j, color::interpolate(scheme.BACKGROUND, scheme.GRID, f));
		}
	for (int i=0;i<6;i++)
		cube_map->overwrite_side(i, im);
}

void drawing_helper_init(const Path &dir) {
	vb_lines = new nix::VertexBuffer("3f,4f");
	vb_2d = new nix::VertexBuffer("3f,4f,2f");
	tex_round = create_round_texture(32);
	tex_text = new nix::Texture();

	nix::Shader::load(dir << "shader/module-surface.shader");
	nix::Shader::load(dir << "shader/module-surface-simple.shader");

	shader_lines_3d = nix::Shader::load(dir << "shader/lines-3d.shader");
	shader_lines_3d_colored = nix::Shader::load(dir << "shader/lines-3d-colored.shader");
	shader_lines_3d_colored_wide = nix::Shader::load(dir << "shader/lines-3d-colored-wide.shader");
	shader_selection = nix::Shader::load(dir << "shader/selection.shader");
	shader_selection->set_int(shader_selection->get_location("num_lights"), 1);

	MultiView::cube_map = new nix::CubeMap(128);
	create_fake_dynamic_cube_map(MultiView::cube_map.get());
}

void set_line_width(float width) {
	if (width == 1.0f) {
		nix::SetShader(shader_lines_3d_colored.get());
	} else {
		auto s = shader_lines_3d_colored_wide.get();
		nix::SetShader(s);
		s->set_float(s->get_location("target_width"), nix::target_width);
		s->set_float(s->get_location("target_height"), nix::target_height);
		s->set_float(s->get_location("line_width"), width);
	}
}

void set_color(const color &c) {
	_cur_color_ = c;
}



void draw_line_2d(float x1, float y1, float x2, float y2, float depth) {
	draw_line(vector(x1, y1, depth), vector(x2, y2, depth));
}

void draw_lines(const Array<vector> &p, bool contiguous) {
	Array<color> c;
	c.resize(p.num);
	for (int i=0; i<c.num; i++)
		c[i] = _cur_color_;
	draw_lines_colored(p, c, contiguous);
}

void draw_lines_colored(const Array<vector> &p, const Array<color> &c, bool contiguous) {
	//set_line_width(2);
	vb_lines->update(0, p);
	vb_lines->update(1, c);
	nix::DrawLines(vb_lines, contiguous);
}

void draw_line(const vector &l1, const vector &l2) {
	draw_lines_colored({l1, l2}, {_cur_color_, _cur_color_}, false);
}


void draw_circle(const vector &pos, const vector &n, float radius) {

	vector e1 = n.ortho();
	vector e2 = n ^ e1;
	e1 *= radius;
	e2 *= radius;
	int N = 64;
	Array<vector> p;
	for (int i=0; i<=N; i++) {
		float w = i * 2 * pi / N;
		p.add(pos + sin(w) * e1 + cos(w) * e2);
	}
	draw_lines(p, true);
}


void draw_helper_line(MultiView::Window *win, const vector &a, const vector &b) {
	nix::SetZ(false, false);
	set_color(scheme.TEXT);
	set_line_width(scheme.LINE_WIDTH_HELPER);
	draw_line(a, b);
	//nix::SetZ(true, true);
	vector pa = win->project(a);
	vector pb = win->project(b);
	//vector d = (pb - pa).normalized();
	//vector e = d ^ vector::EZ;
	float r = 3;
	nix::SetShader(nix::Shader::default_2d);
	draw_rect(pa.x-r, pa.x+r, pa.y-r, pa.y+r, 0);
	draw_rect(pb.x-r, pb.x+r, pb.y-r, pb.y+r, 0);
}


void draw_rect(float x1, float x2, float y1, float y2, float depth) {
	draw_2d(rect::ID, rect(x1, x2, y1, y2), depth);
}

void draw_2d(const rect &src, const rect &dest, float depth) {
	vector a = vector(dest.x1, dest.y1, depth);
	vector b = vector(dest.x2, dest.y1, depth);
	vector c = vector(dest.x1, dest.y2, depth);
	vector d = vector(dest.x2, dest.y2, depth);
	color col = _cur_color_;

	vb_2d->update(0, Array<vector>{a,b,c, c,b,d});
	vb_2d->update(1, Array<color>{col,col,col, col,col,col});
	vb_2d->update(2, Array<float>{src.x1,src.y1, src.x2,src.y1, src.x1,src.y2,  src.x1,src.y2, src.x2,src.y1, src.x2,src.y2});
	nix::DrawTriangles(vb_2d);
}



void draw_round_rect(const rect &r) {

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


int get_str_width(const string &str) {
	Image im;
	render_text(str, im);
	return im.width;
}

void _draw_str(float x, float y, const string &str) {
	Image im;
	render_text(str, im);
	if (im.width > 0) {
		tex_text->overwrite(im);
		SetTexture(tex_text);
		draw_2d(rect::ID, rect(x, x + im.width, y, y + im.height), 0);
	}
}

void draw_str_bg(int x, int y, const string &str, const color &fg, const color &bg, TextAlign align) {
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
	nix::SetTexture(tex_round);
	nix::SetAlpha(ALPHA_MATERIAL);
	float r = scheme.BOX_PADDING;
	set_color(bg);
	draw_round_rect(rect(float(x-r), float(x+wmax+r), float(y-r), float(y+h+r)));
	set_color(fg);
	nix::SetTexture(nullptr);
	nix::SetAlpha(ALPHA_SOURCE_ALPHA, ALPHA_SOURCE_INV_ALPHA);
	foreachi (string &s, xx, i) {
		if (align == TextAlign::RIGHT)
			_draw_str(x+wmax-ww[i], y+line_h*i, s);
		else if (align == TextAlign::CENTER)
			_draw_str(x+wmax/2-ww[i]/2, y+line_h*i, s);
		else if (align == TextAlign::LEFT)
			_draw_str(x, y+line_h*i, s);
	}
	nix::SetAlpha(ALPHA_NONE);
	set_color(c0);
}

void draw_str(int x, int y, const string &str, TextAlign a) {
	color c0 = _cur_color_;
	draw_str_bg(x, y, str, c0, scheme.TEXT_BG, a);
}







void set_font_size(float size) {
	font_size = size;
}

void set_font(const string &name, float size) {
	font_name = name;
	font_size = size;
}


void render_text(const string &text, Image &im) {
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




