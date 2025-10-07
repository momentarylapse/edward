#if HAS_LIB_GL

#include "Painter.h"
#include "Context.h"
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/font.h>
#include "../image/image.h"


namespace ygfx {

void Painter::clear(const color &c) {
	nix::clear(color_input_to_shaders(c));
}

void Painter::draw_str(const vec2 &p, const string &str) {
	if (str.num == 0)
		return;
	Image im;
	face->render_text(str, font::Align::LEFT, im);
	aux->tex_text->write(im);
	aux->tex_text->set_options("minfilter=nearest");
	float w = im.width / ui_scale;
	float h = im.height / ui_scale;
	nix::set_model_matrix(mat4::translation(vec3(offset_x + p.x, offset_y + p.y, 0)) * mat4::scale(w, h, 1));

	nix::set_shader(aux->shader);
	nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
	aux->shader->set_color("_color_", _color);
	aux->shader->set_default_data();
	nix::bind_texture(0, aux->tex_text);
	nix::draw_triangles(aux->vb);
	nix::disable_alpha();
}

void Painter::draw_rect(const rect &r) {
	if (fill) {
		nix::set_model_matrix(mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1));
		auto s = aux->shader;
		if (corner_radius > 0) {
			s = aux->shader_round;
			vec2 size = {r.width(), r.height()};
			s->set_floats("size", &size.x, 2);
			s->set_float("radius", corner_radius);
			s->set_float("softness", softness);
		}
		nix::set_shader(s);
		if (_color.a < 1 or corner_radius > 0) {
			if (accumulate_alpha)
				nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ONE, nix::Alpha::ONE);
			else
				nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
		}
		s->set_color("_color_", _color);
		s->set_default_data();
		nix::bind_texture(0, context->tex_white);
		nix::draw_triangles(aux->vb);
		nix::disable_alpha();
	} else {
		if (corner_radius > 0) {
			draw_line({r.x1 + corner_radius, r.y1}, {r.x2 - corner_radius, r.y1});
			draw_line({r.x1 + corner_radius, r.y2}, {r.x2 - corner_radius, r.y2});
			draw_line({r.x1, r.y1 + corner_radius}, {r.x1, r.y2 - corner_radius});
			draw_line({r.x2, r.y1 + corner_radius}, {r.x2, r.y2 - corner_radius});
			draw_arc({r.x1 + corner_radius, r.y1 + corner_radius}, corner_radius, pi/2, pi);
			draw_arc({r.x2 - corner_radius, r.y1 + corner_radius}, corner_radius, 0, pi/2);
			draw_arc({r.x1 + corner_radius, r.y2 - corner_radius}, corner_radius, -pi/2, -pi);
			draw_arc({r.x2 - corner_radius, r.y2 - corner_radius}, corner_radius, 0, -pi/2);
		} else {
			draw_line({r.x1, r.y1}, {r.x2, r.y1});
			draw_line({r.x1, r.y2}, {r.x2, r.y2});
			draw_line({r.x1, r.y1}, {r.x1, r.y2});
			draw_line({r.x2, r.y1}, {r.x2, r.y2});
		}
	}
}

void Painter::draw_line(const vec2 &a, const vec2 &b) {
	//nix::set_model_matrix(mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1));
	nix::set_model_matrix(mat4::translation(vec3(a.x + offset_x, a.y + offset_y, 0))
			* mat4::rotation_z(atan2(b.y - a.y, b.x - a.x))
			* mat4::scale((b - a).length(), line_width, 1));
	nix::set_shader(aux->shader);
	if (_color.a < 1)
		nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
	aux->shader->set_color("_color_", _color);
	aux->shader->set_default_data();
	nix::bind_texture(0, context->tex_white);
	nix::draw_triangles(aux->vb);
	nix::disable_alpha();
}

void Painter::draw_lines(const Array<vec2> &p) {
	if (contiguous) {
		for (int i=0; i<p.num-1; i++)
			draw_line(p[i], p[i+1]);
	} else {
		for (int i=0; i<p.num-1; i+=2)
			draw_line(p[i], p[i+1]);
	}
}


void Painter::set_transform(float rot[], const vec2 &offset) {
	offset_x = offset.x;
	offset_y = offset.y;
}

void Painter::set_clip(const rect &r) {
	_clip = r;
	nix::set_scissor({r.x1 * ui_scale, max(r.x2, r.x1) * ui_scale, r.y1 * ui_scale, max(r.y2, r.y1) * ui_scale});
}


}

#endif
