/*
 * HDRResolver.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "HDRResolver.h"
#include "ThroughShaderRenderer.h"
#include "../base.h"
#include "../target/TextureRenderer.h"
#include <graphics-impl.h>
#include <Config.h>
#include <helper/PerformanceMonitor.h>
#include <helper/ResourceManager.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>
#include <world/Camera.h>

Any mat4_to_any(const mat4& m);
Any vec2_to_any(const vec2& v);

static float resolution_scale_x = 1.0f;
static float resolution_scale_y = 1.0f;

static int BLUR_SCALE = 4;
static int BLOOM_LEVEL_SCALE = 4;


HDRResolver::HDRResolver(Camera *_cam, const shared<Texture>& tex, const shared<DepthBuffer>& depth_buffer) : Renderer("hdr") {
	cam = _cam;
	tex_main = tex;
	_depth_buffer = depth_buffer;

	int width = tex->width;
	int height = tex->height;


	auto shader_blur = resource_manager->load_shader("forward/blur.shader");
	int bloomw = width, bloomh = height;
	auto bloom_input = tex;
	float r = 3;
	float threshold = 1.0f;
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		auto& bl = bloom_levels[i];
		bloomw /= BLOOM_LEVEL_SCALE;
		bloomh /= BLOOM_LEVEL_SCALE;
		bl.tex_temp = new Texture(bloomw, bloomh, "rgba:f16");
		auto depth0 = new DepthBuffer(bloomw, bloomh, "d:f32");
		bl.tex_out = new Texture(bloomw, bloomh, "rgba:f16");
		auto depth1 = new DepthBuffer(bloomw, bloomh, "d:f32");
		bl.tex_temp->set_options("wrap=clamp");
		bl.tex_out->set_options("wrap=clamp");
		bl.tsr[0] = new ThroughShaderRenderer("blur", shader_blur);
		bl.tsr[0]->bind_texture(0, bloom_input.get());
		bl.tsr[0]->bindings.shader_data.dict_set("axis:0", vec2_to_any(vec2::EX));
		bl.tsr[0]->bindings.shader_data.dict_set("radius:8", r * (float)BLOOM_LEVEL_SCALE);
		bl.tsr[0]->bindings.shader_data.dict_set("threshold:12", threshold);
		bl.tsr[1] = new ThroughShaderRenderer("blur", shader_blur);
		bl.tsr[1]->bind_texture(0, bl.tex_temp.get());
		bl.tsr[1]->bindings.shader_data.dict_set("axis:0", vec2_to_any(vec2::EY));
		bl.tsr[1]->bindings.shader_data.dict_set("radius:8", r);
		bl.tsr[1]->bindings.shader_data.dict_set("threshold:12", 0.0f);
		bl.renderer[0] = new TextureRenderer("blur", {bl.tex_temp, depth0});
		bl.renderer[0]->add_child(bl.tsr[0].get());
		bl.renderer[1] = new TextureRenderer("blur", {bl.tex_out, depth1});
		bl.renderer[1]->add_child(bl.tsr[1].get());
		bloom_input = bl.tex_out;
		threshold = 0;
	}

	auto shader_out = resource_manager->load_shader("forward/hdr.shader");
	out_renderer = new ThroughShaderRenderer("out", shader_out);
	out_renderer->bind_textures(0, {tex.get(), bloom_levels[0].tex_out.get(), bloom_levels[1].tex_out.get(), bloom_levels[2].tex_out.get(), bloom_levels[3].tex_out.get()});
	children.add(out_renderer.get());
}


HDRResolver::~HDRResolver() = default;

void HDRResolver::prepare(const RenderParams& params) {
	PerformanceMonitor::begin(ch_prepare);
	gpu_timestamp_begin(params, ch_prepare);

	for (auto c: children)
		c->prepare(params);

	out_renderer->set_source(dynamicly_scaled_source());


	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		auto& bl = bloom_levels[i];

		bl.tsr[0]->set_source(dynamicly_scaled_source());
		bl.renderer[0]->set_area(dynamicly_scaled_area(bl.renderer[0]->frame_buffer.get()));
		bl.renderer[0]->render(params);

		bl.tsr[1]->set_source(dynamicly_scaled_source());
		bl.renderer[1]->set_area(dynamicly_scaled_area(bl.renderer[1]->frame_buffer.get()));
		bl.renderer[1]->render(params);
	}

	//glGenerateTextureMipmap(fb_small2->color_attachments[0]->texture);

	auto& data = out_renderer->bindings.shader_data;
	data.dict_set("project:128", mat4_to_any(mat4::ID));
	data.dict_set("exposure:192", cam->exposure);
	data.dict_set("bloom_factor:196", cam->bloom_factor);
#ifdef USING_VULKAN
	data.dict_set("gamma:200", 2.2f);
#endif
	data.dict_set("scale_x:204", resolution_scale_x);
	data.dict_set("scale_y:208", resolution_scale_y);

	gpu_timestamp_end(params, ch_prepare);
	PerformanceMonitor::end(ch_prepare);
}

#if 0

/*auto bloom_input = texture_renderer->frame_buffer.get();
float threshold = 1.0f;
for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
	process_blur(cb, bloom_input, bloom_levels[i].fb_temp.get(), threshold, i*2);
	process_blur(cb, bloom_levels[i].fb_temp.get(), bloom_levels[i].fb_out.get(), 0.0f, i*2+1);
	bloom_input = bloom_levels[i].fb_out.get();
	threshold = 0;
}*/

void HDRResolver::process_blur(CommandBuffer *cb, FrameBuffer *source, FrameBuffer *target, float threshold, int iaxis) {
	const vec2 AXIS[2] = {{1,0}, {0,1}};
	//const float SCALE[2] = {(float)BLUR_SCALE, 1};
	//UBOBlur u;
	float radius = (iaxis <= 1) ? 5 : 11;//cam->bloom_radius * resolution_scale_x * 4 / (float)BLUR_SCALE;
	//u.threshold = threshold / cam->exposure;
	//u.axis = AXIS[iaxis % 2];
	//blur_ubo[iaxis]->update(&u);
	//blur_dset[iaxis]->set_uniform_buffer(0, blur_ubo[iaxis]);
	blur_dset[iaxis]->set_texture(1, source->attachments[0].get());
	blur_dset[iaxis]->update();

	auto rp = blur_render_pass[iaxis];

	cb->begin_render_pass(rp, target);
	cb->set_viewport(dynamicly_scaled_area(target));

	cb->bind_pipeline(blur_pipeline[iaxis / 2]);
	cb->bind_descriptor_set(0, blur_dset[iaxis]);

	Any axis_x, axis_y;
	axis_x.list_set(0, 1.0f);
	axis_x.list_set(1, 0.0f);
	axis_y.list_set(0, 0.0f);
	axis_y.list_set(1, 1.0f);

	Any data;
	data.dict_set("radius:8", radius);
	data.dict_set("threshold:12", threshold / cam->exposure);
	if ((iaxis % 2) == 0)
		data.dict_set("axis:0", axis_x);
	else
		data.dict_set("axis:0", axis_y);

	apply_shader_data(cb, data);

	cb->draw(vb_2d.get());

	cb->end_render_pass();

	//process(cb, {source->attachments[0].get()}, target, shader_blur.get());
}
#endif

