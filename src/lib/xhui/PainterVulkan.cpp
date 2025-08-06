#if HAS_LIB_VULKAN

#include "Painter.h"
#include "Context.h"
#include "Theme.h"
#include "draw/font.h"

#include "../vulkan/vulkan.h"
#include "../image/image.h"
#include "../math/mat4.h"
#include <lib/base/algo.h>
#include "../os/msg.h"


using namespace vulkan;


namespace xhui {

mat4 mat_pixel_to_rel;

Array<DescriptorSet*> descriptor_sets;
int descriptor_sets_used = 0;

DescriptorSet* get_descriptor_set(Context* context, Texture* texture) {
	DescriptorSet* dset = nullptr;
	if (descriptor_sets_used < descriptor_sets.num) {
		dset = descriptor_sets[descriptor_sets_used ++];
	} else {
		dset = context->pool->create_set(context->shader);
		descriptor_sets.add(dset);
	}
	dset->set_texture(0, texture);
	dset->update();
	return dset;
}

struct Parameters {
	mat4 matrix;
	color col;
	vec2 size;
	float radius, softness;
};

Painter::Painter(Window *w) {
	window = w;
	if (window) {
		ui_scale = window->ui_scale;
		context = window->context;
		face = default_font_regular;

		context->start();

		Painter::set_color(Theme::_default.text);
		Painter::set_font(Theme::_default.font_name /*"CAC Champagne"*/, Theme::_default.font_size, false, false);

		cb = context->current_command_buffer();
		auto fb = context->current_frame_buffer();


		cb->begin();

		width = (int)((float)context->swap_chain->width / ui_scale);
		height = (int)((float)context->swap_chain->height / ui_scale);
		mat_pixel_to_rel = mat4::translation({- 1,- 1, 0}) *  mat4::scale(2.0f / (float)width, 2.0f / (float)height, 1);

		_area = {0, (float)width, 0, (float)height};
		native_area = {0, (float)context->swap_chain->width, 0, (float)context->swap_chain->height};
		native_area_window = native_area;
		_clip = _area;

		window->handle_event_p(window->id, event_id::JustBeforeDraw, this);

		cb->set_viewport(native_area);
		cb->begin_render_pass(context->render_pass, fb);
		cb->set_scissor(native_area);
		cb->clear(native_area, {Black}, 1);
	}
}

void Painter::end() {

	cb->end_render_pass();
	cb->end();


	auto f = context->wait_for_frame_fences[context->image_index];
	context->device->present_queue.submit(cb, {context->image_available_semaphore}, {context->render_finished_semaphore}, f);

	context->swap_chain->present(context->image_index, {context->render_finished_semaphore});

	context->device->wait_idle();

	descriptor_sets_used = 0;

	iterate_text_caches();
}

void Painter::clear(const color &c) {
	cb->clear(native_area, {c}, 1);
}

void Painter::draw_str(const vec2 &p, const string &str) {
	if (str.num == 0)
		return;
	auto& tc = get_text_cache(context, str, face, font_size, ui_scale);

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

	cb->bind_pipeline(context->pipeline_alpha);
	cb->push_constant(0, sizeof(params), &params);

	cb->bind_descriptor_set(0, tc.dset);
	cb->draw(context->vb);
}


void fill_rect(Context* context, const rect& r, const color& _color, float radius, float softness) {
	Parameters params;
	params.matrix = mat_pixel_to_rel * mat4::translation({r.x1, r.y1, 0}) *  mat4::scale(r.width(), r.height(), 1);
	params.col = _color;
	params.size = {r.width(), r.height()};
	params.radius = radius;
	params.softness = softness;

	auto cb = context->current_command_buffer();
	if (radius > 0 or softness > 0 or _color.a < 1)
		cb->bind_pipeline(context->pipeline_alpha);
	else
		cb->bind_pipeline(context->pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, context->dset);
	cb->draw(context->vb);
}

void Painter::draw_rect(const rect &r) {
	if (fill) {
		fill_rect(context, r, _color, corner_radius, softness);
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

		auto vb = context->get_line_vb();
		Array<Vertex1> p;
		add_vb_line(p, a, b, line_width);
		vb->update(p);
		Parameters params;
		params.matrix = mat_pixel_to_rel;
		params.col = _color;
		params.size = {(float)width, (float)height};
		params.radius = 0;//line_width;
		params.softness = 0;//softness;

		auto cb = context->current_command_buffer();
		cb->bind_pipeline(context->pipeline);
		cb->push_constant(0, sizeof(params), &params);
		cb->bind_descriptor_set(0, context->dset);
		cb->draw(vb);
	//}
}

void Painter::draw_lines(const Array<vec2> &p) {
	/*for (int i=0; i<p.num-1; i++)
		draw_line(p[i], p[i+1]);*/

	auto vb = context->get_line_vb();
	Array<Vertex1> vertices;
	for (int i=0; i<p.num-1; i++)
		add_vb_line(vertices, p[i], p[i+1], line_width);
	vb->update(vertices);
	Parameters params;
	params.matrix = mat_pixel_to_rel;
	params.col = _color;
	params.size = {(float)width, (float)height};
	params.radius = 0;//line_width;
	params.softness = 0;//softness;

	auto cb = context->current_command_buffer();
	cb->bind_pipeline(context->pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, context->dset);
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



void Painter::draw_ximage(const rect& r, const XImage* image) {
	auto dset = get_descriptor_set(context, image->texture.get());
	Parameters params;
	params.matrix = mat_pixel_to_rel * mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1);
	params.col = _color;
	params.size = {r.width(),r.height()};
	params.radius = 0;
	params.softness = 0;

	cb->bind_pipeline(context->pipeline_alpha);
	cb->push_constant(0, sizeof(params), &params);

	cb->bind_descriptor_set(0, dset);
	cb->draw(context->vb);
}


}

#endif
