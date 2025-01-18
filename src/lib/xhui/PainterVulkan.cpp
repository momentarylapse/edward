#if HAS_LIB_VULKAN

#include "Painter.h"
#include "ContextVulkan.h"
#include "Theme.h"
#include "draw/font.h"

#include "../vulkan/vulkan.h"
#include "../image/image.h"
#include "../math/mat4.h"
#include "../os/msg.h"


using namespace vulkan;


namespace xhui {



CommandBuffer* cb;

struct TextCache {
	string text;
	float size;
	// font...
	Texture* texture;
	DescriptorSet* dset;
};

mat4 mat_pixel_to_rel;

Array<TextCache> text_caches;
int text_caches_used = 0;

TextCache& get_text_cache(ContextVulkan* context) {
	if (text_caches_used < text_caches.num)
		return text_caches[text_caches_used ++];
	TextCache tc;
	tc.dset = context->pool->create_set(context->shader);
	tc.texture = new Texture();
	text_caches.add(tc);
	text_caches_used ++;
	return text_caches.back();
}

struct Parameters {
	mat4 matrix;
	color col;
};

Painter::Painter(Window *w) {
	window = w;
	context = window->context;

	context->start();

	Painter::set_color(Theme::_default.text);
	Painter::set_font(Theme::_default.font_name /*"CAC Champagne"*/, Theme::_default.font_size, false, false);
	
	cb = context->command_buffers[context->image_index];
	auto fb = context->frame_buffers[context->image_index];
	
	
	cb->begin();

	width = (float)context->swap_chain->width / ui_scale;
	height = (float)context->swap_chain->height / ui_scale;
	mat_pixel_to_rel = mat4::translation({- 1,- 1, 0}) *  mat4::scale(2.0f / (float)width, 2.0f / (float)height, 1);

	rect area0 = {0, (float)context->swap_chain->width, 0, (float)context->swap_chain->height};
	cb->set_viewport(area0);
	cb->begin_render_pass(context->render_pass, fb);
	cb->clear(area0, {Black}, 1);
}

void Painter::end() {

	cb->end_render_pass();
	cb->end();


	auto f = context->wait_for_frame_fences[context->image_index];
	context->device->present_queue.submit(cb, {context->image_available_semaphore}, {context->render_finished_semaphore}, f);

	context->swap_chain->present(context->image_index, {context->render_finished_semaphore});

	context->device->wait_idle();

	text_caches_used = 0;
}

void Painter::clear(const color &c) {
	rect area0 = {0, (float)context->swap_chain->width, 0, (float)context->swap_chain->height};
	cb->clear(area0, {c}, 1);
}

void Painter::set_font(const string &font, float size, bool bold, bool italic) {
	font_name = font;
	font_size = size;
	font::set_font(font, size * ui_scale);
}

void Painter::set_font_size(float size) {
	font_size = size;
	font::set_font(font_name, font_size * ui_scale);
}

void Painter::set_color(const color &c) {
	_color = c;
}

void Painter::draw_str(const vec2 &p, const string &str) {
	if (str.num == 0)
		return;
	Image im;
	font::render_text(str, Align::LEFT, im);
	auto& tc = get_text_cache(context);
	tc.texture->write(im);
	tc.texture->set_options("minfilter=nearest");
	float w = im.width / ui_scale;
	float h = im.height / ui_scale;
	Parameters params;
	params.matrix = mat_pixel_to_rel * mat4::translation(vec3(offset_x + p.x, offset_y + p.y, 0)) * mat4::scale(w, h, 1);
	params.col = _color;

	cb->bind_pipeline(context->pipeline_alpha);
	cb->push_constant(0, sizeof(params), &params);

	tc.dset->set_texture(0, tc.texture);
	tc.dset->update();
	cb->bind_descriptor_set(0, tc.dset);
	cb->draw(context->vb);
}

vec2 Painter::get_str_size(const string &str) {
	return {font::get_text_width(str) * ui_scale, font_size * ui_scale};
}

void Painter::set_line_width(float width) {
	line_width = width;
}

void Painter::set_roundness(float radius) {
	corner_radius = radius;
}

void Painter::draw_arc(const vec2& p, float r, float w0, float w1) {
	//float w = (w0 + w1) / 2;
	draw_line({p.x + r * cos(w0), p.y - r * sin(w0)}, {p.x + r * cos(w1), p.y - r * sin(w1)});
	//draw_line({p.x + r * cos(w), p.y - r * sin(w)}, {p.x + r * cos(w1), p.y - r * sin(w1)});
}

void fill_rect(ContextVulkan* context, const rect& r, const color& _color) {
	Parameters params;
	params.matrix = mat_pixel_to_rel * mat4::translation({r.x1, r.y1, 0}) *  mat4::scale(r.width(), r.height(), 1);
	params.col = _color;

	cb->bind_pipeline(context->pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, context->dset);
	cb->draw(context->vb);
}

void Painter::draw_rect(const rect &r) {
	if (fill) {
		fill_rect(context, r, _color);
	} else {
		draw_line({r.x1, r.y1}, {r.x2, r.y1});
		draw_line({r.x1, r.y2}, {r.x2, r.y2});
		draw_line({r.x1, r.y1}, {r.x1, r.y2});
		draw_line({r.x2, r.y1}, {r.x2, r.y2});
	}
}

void Painter::draw_line(const vec2 &a, const vec2 &b) {
	if (a.x == b.x)
		fill_rect(context, rect(a.x, a.x + 1, a.y, b.y), _color);
	else if (a.y == b.y)
		fill_rect(context, rect(a.x, b.x, a.y, a.y+1), _color);
}

void Painter::draw_lines(const Array<vec2> &p) {
	for (int i=0; i<p.num-1; i++)
		draw_line(p[i], p[i+1]);
}


void Painter::set_transform(float rot[], const vec2 &offset) {
	return;
	offset_x = offset.x;
	offset_y = offset.y;
}

void Painter::set_clip(const rect &r) {
}

}

#endif
