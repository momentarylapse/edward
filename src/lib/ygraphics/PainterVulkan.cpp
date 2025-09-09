#if HAS_LIB_VULKAN

#include "Painter.h"
#include "Context.h"
#include <lib/ygraphics/font.h>
#include "../vulkan/vulkan.h"
#include "../image/image.h"
#include "../math/mat4.h"
#include <lib/base/algo.h>
#include "../os/msg.h"

using namespace vulkan;


namespace ygfx {

DescriptorSet* get_descriptor_set(Context* context, Texture* texture);

struct Parameters {
	mat4 matrix;
	color col;
	vec2 size;
	float radius, softness;
};

void Painter::clear(const color &c) {
	cb->clear(native_area, {c}, 1);
}

void Painter::draw_str(const vec2 &p, const string &str) {
	if (str.num == 0)
		return;
	auto& tc = aux->get_text_cache(str, face, font_size, ui_scale);

	float w = (float)tc.texture->width / ui_scale;
	float h = (float)tc.texture->height / ui_scale;
	Parameters params;
	params.matrix = mat_pixel_to_rel * mat4::translation(vec3(offset_x + p.x, offset_y + p.y, 0)) * mat4::scale(w, h, 1);
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


void fill_rect(Painter* p, const rect& r, const color& _color, float radius, float softness) {
	Parameters params;
	params.matrix = p->mat_pixel_to_rel * mat4::translation({r.x1, r.y1, 0}) *  mat4::scale(r.width(), r.height(), 1);
	params.col = _color;
	params.size = {r.width(), r.height()};
	params.radius = radius;
	params.softness = softness;

	auto cb = p->cb;
	if (radius > 0 or softness > 0 or _color.a < 1)
		cb->bind_pipeline(p->aux->pipeline_alpha);
	else
		cb->bind_pipeline(p->aux->pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, p->aux->dset);
	cb->draw(p->aux->vb);
}

void Painter::draw_rect(const rect &r) {
	if (fill) {
		fill_rect(this, r, _color, corner_radius, softness);
	} else {
		draw_line({r.x1, r.y1}, {r.x2, r.y1});
		draw_line({r.x1, r.y2}, {r.x2, r.y2});
		draw_line({r.x1, r.y1}, {r.x1, r.y2});
		draw_line({r.x2, r.y1}, {r.x2, r.y2});
	}
}

static void add_vb_line(Array<Vertex1>& vertices, const vec2& a, const vec2& b, float line_width) {
	vec2 dir = (b - a).normalized();
	vec2 r = dir.ortho() * line_width / 2;
	dir *= line_width * 0.2f;
	vec2 a0 = a - r - dir;
	vec2 a1 = a + r - dir;
	vec2 b0 = b - r + dir;
	vec2 b1 = b + r + dir;
	vertices.add({{a0.x, a0.y, 0}, v_0, 0,0});
	vertices.add({{a1.x, a1.y, 0}, v_0, 0,0});
	vertices.add({{b0.x, b0.y, 0}, v_0, 0,0});
	vertices.add({{b0.x, b0.y, 0}, v_0, 0,0});
	vertices.add({{a1.x, a1.y, 0}, v_0, 0,0});
	vertices.add({{b1.x, b1.y, 0}, v_0, 0,0});
}

void Painter::draw_line(const vec2 &a, const vec2 &b) {
	/*if (a.x == b.x) {
		fill_rect(context, rect(a.x + 0.5f - line_width/2, a.x + 0.5f + line_width/2, a.y, b.y), _color, 0, 0);
	} else if (a.y == b.y) {
		fill_rect(context, rect(a.x, b.x, a.y + 0.5f - line_width/2, a.y + 0.5f + line_width/2), _color, 0, 0);
	} else {*/
		// NO geometry shaders on M1... :(
		// CPU lines then...

		auto vb = aux->get_line_vb();
		Array<Vertex1> p;
		add_vb_line(p, a, b, line_width);
		vb->update(p);
		Parameters params;
		params.matrix = mat_pixel_to_rel;
		params.col = _color;
		params.size = {(float)width, (float)height};
		params.radius = 0;//line_width;
		params.softness = 0;//softness;

		cb->bind_pipeline(aux->pipeline);
		cb->push_constant(0, sizeof(params), &params);
		cb->bind_descriptor_set(0, aux->dset);
		cb->draw(vb);
	//}
}

void Painter::draw_lines(const Array<vec2> &p) {
	/*for (int i=0; i<p.num-1; i++)
		draw_line(p[i], p[i+1]);*/

	auto vb = aux->get_line_vb();
	Array<Vertex1> vertices;
	if (contiguous) {
		for (int i=0; i<p.num-1; i++)
			add_vb_line(vertices, p[i], p[i+1], line_width);
	} else {
		for (int i=0; i<p.num-1; i+=2)
			add_vb_line(vertices, p[i], p[i+1], line_width);
	}
	vb->update(vertices);
	Parameters params;
	params.matrix = mat_pixel_to_rel;
	params.col = _color;
	params.size = {(float)width, (float)height};
	params.radius = 0;//line_width;
	params.softness = 0;//softness;

	cb->bind_pipeline(aux->pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, aux->dset);
	cb->draw(vb);
}


void Painter::set_transform(float rot[], const vec2 &offset) {
	return;
	offset_x = offset.x;
	offset_y = offset.y;
}

void Painter::set_clip(const rect &r) {
	_clip = r;
	cb->set_scissor({r.x1 * ui_scale, r.x2 * ui_scale, r.y1 * ui_scale, r.y2 * ui_scale});
}


}

#endif
