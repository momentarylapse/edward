#if HAS_LIB_GL

#include "Painter.h"
#include "Context.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/image/image.h>

#include "TextCache.h"


namespace ygfx {


void draw_simple(DrawingHelperData* aux, const Array<VertexX>& p, const mat4& mat, const color& col, bool use_z, bool use_blending) {
	auto vb = aux->get_line_vb(true);
	vb->update(p);

	auto s = aux->shader;

	nix::set_shader(s);
	nix::set_z(use_z and !use_blending, use_z);
	if (use_blending)
		nix::set_alpha(Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA);
	nix::set_cull(CullMode::NONE);
	s->set_color("color", col);
	if (aux->projection_matrix)
		s->set_matrix("matrix", *aux->projection_matrix * mat);
	else
		s->set_matrix("matrix", mat);
	nix::bind_texture(0, aux->context->tex_white);
	nix::draw_triangles(vb);
	nix::disable_alpha();
}

void Painter::clear(const color &c) {
	nix::clear(context->color_input_to_shaders(c));
}

void Painter::draw_str(const vec2 &p, const string &str) {
	if (str.num == 0)
		return;
	auto& tc = text_cache->get(str, face, font_size, ui_scale.y);
	float w = (float)tc.texture->width / ui_scale.x;
	float h = (float)tc.texture->height / ui_scale.y;
	const auto mat = mat4::translation(vec3(p + offset, 0)) * mat4::scale(w, h, 1);

	auto s = aux->shader;
	nix::set_shader(s);
	nix::set_alpha_split(Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, Alpha::ZERO, Alpha::ONE);
	s->set_color("color", _color);
	s->set_matrix("matrix", mat_pixel_to_rel * mat);
	nix::bind_texture(0, tc.texture);
	nix::draw_triangles(aux->vb);
	nix::disable_alpha();
}

void Painter::draw_rect(const rect &r) {
	if (fill) {
		const auto mat = mat4::translation(vec3(r.p00() + offset, 0)) * mat4::scale(r.width(), r.height(), 1);
		auto s = aux->shader;
		if (corner_radius > 0) {
			s = aux->shader_round;
			vec2 size = {r.width() * ui_scale.x, r.height() * ui_scale.y};
			s->set_floats("size", &size.x, 2);
			s->set_float("radius", corner_radius * ui_scale.y);
			s->set_float("softness", softness);
		}
		if (user_shader)
			s = user_shader;
		nix::set_shader(s);
		if (_color.a < 1 or corner_radius > 0 or user_texture) {
			if (accumulate_alpha)
				nix::set_alpha_split(Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, Alpha::ONE, Alpha::ONE);
			else
				nix::set_alpha_split(Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, Alpha::ZERO, Alpha::ONE);
		}
		s->set_color("color", _color);
		s->set_matrix("matrix", mat_pixel_to_rel * mat);
		if (user_texture)
			nix::bind_texture(0, user_texture);
		else
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


void Painter::set_transform(float rot[], const vec2& _offset) {
	offset = _offset;
}

void Painter::set_clip(const rect &r) {
	_clip = r;
	nix::set_scissor({r.x1 * ui_scale.x, max(r.x2, r.x1) * ui_scale.x, r.y1 * ui_scale.y, max(r.y2, r.y1) * ui_scale.y}, native_area_window);
}


}

#endif
