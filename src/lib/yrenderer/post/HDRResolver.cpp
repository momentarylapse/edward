/*
 * HDRResolver.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "HDRResolver.h"
#include <lib/yrenderer/post/ThroughShaderRenderer.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/target/TextureRenderer.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/ShaderManager.h>
#include <lib/profiler/Profiler.h>
#include <lib/math/vec2.h>
#include <lib/math/mat4.h>
#include <lib/os/msg.h>

Any mat4_to_any(const mat4& m);
Any vec2_to_any(const vec2& v);

namespace yrenderer {

static int BLUR_SCALE = 4;
static int BLOOM_LEVEL_SCALE = 4;

string HDRResolver::magfilter = "nearest";

HDRResolver::HDRResolver(Context* ctx) : Renderer(ctx, "hdr") {

	_create_textures(16, 16);

	auto shader_blur = REQUIRED(shader_manager->load_shader("post/blur.shader"));
	auto bloom_input = texture;
	float r = 3;
	float threshold = 1.0f;
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		auto& bl = bloom_levels[i];
		bl.tsr[0] = new ThroughShaderRenderer(ctx, "blur", shader_blur);
		bl.tsr[0]->bindings.shader_data.dict_set("axis:0", vec2_to_any(vec2::EX));
		bl.tsr[0]->bindings.shader_data.dict_set("radius:8", Any(r * (float)BLOOM_LEVEL_SCALE));
		bl.tsr[0]->bindings.shader_data.dict_set("threshold:12", Any(threshold));
		bl.tsr[1] = new ThroughShaderRenderer(ctx, "blur", shader_blur);
		bl.tsr[1]->bindings.shader_data.dict_set("axis:0", vec2_to_any(vec2::EY));
		bl.tsr[1]->bindings.shader_data.dict_set("radius:8", Any(r));
		bl.tsr[1]->bindings.shader_data.dict_set("threshold:12", Any(0.0f));
		bl.renderer[0] = new TextureRenderer(ctx, "blur", {bl.tex_temp, bl.depth_temp});
		bl.renderer[0]->add_child(bl.tsr[0].get());
		add_sub_task(bl.renderer[0].get());
		bl.renderer[1] = new TextureRenderer(ctx, "blur", {bl.tex_out, bl.depth_out});
		bl.renderer[1]->add_child(bl.tsr[1].get());
		add_sub_task(bl.renderer[1].get());
		bloom_input = bl.tex_out;
		threshold = 0;
	}

	auto shader_out = REQUIRED(shader_manager->load_shader("post/hdr.shader"));
	out_renderer = new ThroughShaderRenderer(ctx, "out", shader_out);
	add_child(out_renderer.get());

	auto ddd = (ygfx::Texture*)depth_buffer.get();
	texture_renderer = new TextureRenderer(ctx, "tex", {texture, ddd});
	add_sub_task(texture_renderer.get());
	_update_textures();
	_set_resolution(max_width, max_height);

	custodian = texture_renderer.get();
}


HDRResolver::~HDRResolver() = default;

void HDRResolver::set_resolution(int w, int h) {
	_set_resolution(w, h);
	user_resolution = true;
}

void HDRResolver::_set_resolution(int w, int h) {
	if (w > max_width or h > max_height) {
		_create_textures(max(w, max_width), max(h, max_height));
		_update_textures();
	}
	current_width = min(w, max_width);
	current_height = min(h, max_height);
	resolution_scale.x = (float)w / (float)max_width;
	resolution_scale.y = (float)h / (float)max_height;
}

void HDRResolver::_create_textures(int width, int height) {
	max_width = width;
	max_height = height;
	texture = new ygfx::Texture(width, height, "rgba:f16");
	texture->set_options("wrap=clamp,minfilter=nearest");
	texture->set_options("magfilter=" + magfilter);
	depth_buffer = new ygfx::DepthBuffer(width, height, "d:f32");

	int bloomw = width, bloomh = height;
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		auto& bl = bloom_levels[i];
		bloomw = max(bloomw / BLOOM_LEVEL_SCALE, 4);
		bloomh = max(bloomh / BLOOM_LEVEL_SCALE, 4);
		bl.tex_temp = new ygfx::Texture(bloomw, bloomh, "rgba:f16");
		bl.depth_temp = new ygfx::DepthBuffer(bloomw, bloomh, "d:f32");
		bl.tex_out = new ygfx::Texture(bloomw, bloomh, "rgba:f16");
		bl.depth_out = new ygfx::DepthBuffer(bloomw, bloomh, "d:f32");
		bl.tex_temp->set_options("wrap=clamp");
		bl.tex_out->set_options("wrap=clamp");
	}
}

void HDRResolver::_update_textures() {

	auto bloom_input = texture;
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		auto& bl = bloom_levels[i];
		bl.tsr[0]->bind_texture(0, bloom_input.get());
		bl.tsr[1]->bind_texture(0, bl.tex_temp.get());
		bl.renderer[0]->set_textures({bl.tex_temp, bl.depth_temp});
		bl.renderer[1]->set_textures({bl.tex_out, bl.depth_out});
		bloom_input = bl.tex_out;
	}

	out_renderer->bind_textures(0, {texture.get(), bloom_levels[0].tex_out.get(), bloom_levels[1].tex_out.get(), bloom_levels[2].tex_out.get(), bloom_levels[3].tex_out.get()});

	auto ddd = (ygfx::Texture*)depth_buffer.get();
	texture_renderer->set_textures({texture, ddd});
}

void HDRResolver::prepare(const RenderParams& params) {
	profiler::begin(ch_prepare);
	ctx->gpu_timestamp_begin(params, ch_prepare);

	if (!user_resolution)
		_set_resolution((int)params.area.width(), (int)params.area.height());

	if (texture_renderer) {
		texture_renderer->set_area({0,(float)current_width, 0, (float)current_height});
		texture_renderer->render(params);
	} else {
		for (auto c: children)
			c->prepare(params);
	}

	out_renderer->prepare(params);

	out_renderer->set_source(dynamicly_scaled_source(resolution_scale));


	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		auto& bl = bloom_levels[i];

		bl.tsr[0]->set_source(dynamicly_scaled_source(resolution_scale));
		bl.renderer[0]->set_area(dynamicly_scaled_area(bl.renderer[0]->frame_buffer.get(), resolution_scale));
		bl.renderer[0]->render(params);

		bl.tsr[1]->set_source(dynamicly_scaled_source(resolution_scale));
		bl.renderer[1]->set_area(dynamicly_scaled_area(bl.renderer[1]->frame_buffer.get(), resolution_scale));
		bl.renderer[1]->render(params);
	}

	//glGenerateTextureMipmap(fb_small2->color_attachments[0]->texture);

	auto& data = out_renderer->bindings.shader_data;
	data.dict_set("exposure:0", Any(exposure));
	data.dict_set("bloom_factor:4", Any(bloom_factor));
#ifdef USING_VULKAN
	data.dict_set("gamma:8", Any(2.2f));
#endif
	data.dict_set("scale_x:12", Any(1.0f));//resolution_scale.x));
	data.dict_set("scale_y:16", Any(1.0f));//resolution_scale.y));

	ctx->gpu_timestamp_end(params, ch_prepare);
	profiler::end(ch_prepare);
}


}

