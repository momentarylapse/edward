#if HAS_LIB_VULKAN

#include "Painter.h"
#include "Context.h"
#include <lib/ygraphics/font.h>
#include <lib/vulkan/vulkan.h>
#include <lib/image/image.h>
#include <lib/math/mat4.h>
#include <lib/base/algo.h>
#include <lib/os/msg.h>
#include <cmath>

#include "TextCache.h"

using namespace vulkan;


namespace ygfx {

struct Parameters {
	mat4 matrix;
	color col;
	vec2 size;
	float radius, softness;
};

void draw_simple(DrawingHelperData* aux, const Array<VertexX>& p, const mat4& mat, const color& col, bool use_z, bool use_blending) {
	auto vb = aux->get_line_vb(true);
	vb->update(p);
	Parameters params;
	if (aux->projection_matrix)
		params.matrix = *aux->projection_matrix * mat;
	else
		params.matrix = mat;
	params.size = {(float)1000, (float)1000};
	params.col = col;
	params.radius = 0;//line_width;
	params.softness = 0;//softness;

	auto cb = aux->cb;
	cb->bind_pipeline(use_blending ? aux->pipeline_alpha : (use_z ? aux->pipeline_z : aux->pipeline));
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, aux->dset);
	cb->draw(vb);
}

void Painter::clear(const color &c) {
	cb->clear(native_area, {context->color_input_to_shaders(c)}, 1);
}

void Painter::draw_str(const vec2 &p, const string &str) {
	if (str.num == 0)
		return;
	auto& tc = text_cache->get(str, face, font_size, ui_scale.y);
	bool round_to_pixels = true;

	float w = (float)tc.texture->width / ui_scale.x;
	float h = (float)tc.texture->height / ui_scale.y;
	Parameters params;
	vec2 q = p + offset;
	if (round_to_pixels) {
		float fx = native_area.width() / _area.width();
		float fy = native_area.height() / _area.height();
		w = (float)tc.texture->width / fx;
		h = (float)tc.texture->height / fy;
		q.x = std::roundf(q.x * fx) / fx;
		q.y = std::roundf(q.y * fy) / fy;
	}
	params.matrix = mat_pixel_to_rel * mat4::translation(vec3(q, 0)) * mat4::scale(w, h, 1);
	params.col = _color;
	params.size = {w,h};
	params.radius = 0;
	params.softness = 0;

	tc.dset->set_texture(0, tc.texture);
	tc.dset->update();

	cb->bind_pipeline(aux->pipeline_alpha);
	cb->push_constant(0, sizeof(params), &params);

	cb->bind_descriptor_set(0, tc.dset);
	cb->draw(aux->vb);
}


void fill_rect(Painter* p, const rect& r, const color& _color, float radius, float softness, vulkan::DescriptorSet* dset, bool force_blending = false) {
	Parameters params;
	params.matrix = p->mat_pixel_to_rel * mat4::translation({r.x1, r.y1, 0}) *  mat4::scale(r.width(), r.height(), 1);
	params.col = _color;
	params.size = r.size() * p->ui_scale.y;
	params.radius = radius * p->ui_scale.y;
	params.softness = softness;

	auto cb = p->cb;
	if (radius > 0 or softness > 0)
		cb->bind_pipeline(p->aux->pipeline_round);
	else if (force_blending or _color.a < 1)
		cb->bind_pipeline(p->aux->pipeline_alpha);
	else
		cb->bind_pipeline(p->aux->pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, dset);
	cb->draw(p->aux->vb);
}

void Painter::draw_rect(const rect& r) {
	if (fill) {
		if (user_texture) {
			auto dset = aux->get_descriptor_set(user_texture);
			fill_rect(this, r, _color, corner_radius, softness, dset, true);
		} else {
			fill_rect(this, r, _color, corner_radius, softness, aux->dset);
		}
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
	cb->set_scissor({r.x1 * ui_scale.x, r.x2 * ui_scale.x, r.y1 * ui_scale.y, r.y2 * ui_scale.y});
}


}

#endif
