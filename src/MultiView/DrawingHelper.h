/*
 * DrawingHelper.h
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"

namespace nix {
	class Shader;
	class Texture;
	class CubeMap;
}

extern shared<nix::Shader> shader_selection;
extern shared<nix::Shader> shader_lines_3d;
extern shared<nix::Shader> shader_lines_3d_colored;
extern shared<nix::Shader> shader_lines_3d_colored_wide;
extern shared<nix::Texture> tex_white;

class vec3;
namespace MultiView {
	class Window;
	extern shared<nix::CubeMap> cube_map;
}
class string;
class color;
class Image;
class rect;
class Path;

void drawing_helper_init(const Path &dir);

void set_color(const color &c);
void set_line_width(float width);
void draw_helper_line(MultiView::Window *win, const vec3 &a, const vec3 &b);
void draw_circle(const vec3 &pos, const vec3 &n, float radius);


void draw_rect(float x1, float x2, float y1, float y2, float depth);
void draw_2d(const rect &src, const rect &dest, float depth);

void draw_line_2d(float x1, float y1, float x2, float y2, float depth);
void draw_line(const vec3 &l1, const vec3 &l2);
void draw_lines(const Array<vec3> &p, bool contiguous);
void draw_lines_colored(const Array<vec3> &p, const Array<color> &c, bool contiguous);

enum class TextAlign {
	LEFT,
	CENTER,
	RIGHT
};
void draw_str_bg(int x, int y, const string &str, const color &fg, const color &bg, TextAlign align);
void draw_str(int x, int y, const string &str, TextAlign a = TextAlign::LEFT);
int get_str_width(const string &str);
void set_font_size(float size);
void set_font(const string &name, float size);

void render_text(const string &text, Image &im);



void set_material_selected();
void set_material_hover();
void set_material_creation(float intensity = 1.0f);
