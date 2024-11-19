/*
 * HDRRendererVulkan.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "HDRRendererVulkan.h"
#ifdef USING_VULKAN
#include "../base.h"
#include "../helper/ComputeTask.h"
#include <graphics-impl.h>
#include <Config.h>
#include <helper/PerformanceMonitor.h>
#include <helper/ResourceManager.h>
#include <lib/base/iter.h>
#include <lib/math/rect.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>
#include <world/Camera.h>
#include <world/World.h>

struct UBOBlur{
	vec2 axis;
	//float dummy1[2];
	float radius;
	float threshold;
	float dummy2[2];
	float kernel[20];
};

UniformBuffer *blur_ubo[HDRRendererVulkan::MAX_BLOOM_LEVELS*2];
DescriptorSet *blur_dset[HDRRendererVulkan::MAX_BLOOM_LEVELS*2];
GraphicsPipeline *blur_pipeline[HDRRendererVulkan::MAX_BLOOM_LEVELS];
RenderPass *blur_render_pass[HDRRendererVulkan::MAX_BLOOM_LEVELS*2];

static float resolution_scale_x = 1.0f;
static float resolution_scale_y = 1.0f;

static int BLUR_SCALE = 4;
static int BLOOM_HEIGHT0 = 256;

HDRRendererVulkan::RenderOutData::RenderOutData(Shader *s, const Array<Texture*> &tex) {
	shader_out = s;
	dset_out = pool->create_set("buffer,sampler,sampler,sampler,sampler,sampler");

	for (auto&& [i, t]: enumerate(tex))
		dset_out->set_texture(1 + i, t);
	dset_out->update();

	vb_2d = new VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);
	vb_2d_current_source = rect::ID_SYM;
}

void HDRRendererVulkan::RenderOutData::render_out(CommandBuffer *cb, const Array<float> &data, float exposure, const RenderParams& params) {
	auto source = dynamicly_scaled_source();
	if (source != vb_2d_current_source) {
		vb_2d->create_quad(rect::ID_SYM, source);
		vb_2d_current_source = source;
	}

	if (!pipeline_out) {
		pipeline_out = new vulkan::GraphicsPipeline(shader_out.get(), params.render_pass, 0, "triangles", "3f,3f,2f");
		pipeline_out->set_culling(CullMode::NONE);
		pipeline_out->set_z(false, false);
		pipeline_out->rebuild();
	}

	cb->bind_pipeline(pipeline_out);
	cb->bind_descriptor_set(0, dset_out);
	struct PCOut {
		mat4 p, m, v;
		float x[32];
	};
	PCOut pco = {mat4::ID, mat4::ID, mat4::ID, exposure};
	memcpy(&pco.x, &data[0], sizeof(float) * data.num);
	cb->push_constant(0, sizeof(mat4) * 3 + sizeof(float) * data.num, &pco);
	cb->draw(vb_2d);
}


HDRRendererVulkan::RenderIntoData::RenderIntoData(int width, int height) {
	auto tex = new vulkan::Texture(width, height, "rgba:f16");
	tex->set_options("wrap=clamp,magfilter=" + config.resolution_scale_filter);
	_depth_buffer = new DepthBuffer(width, height, "d:f32", true);
	_render_pass = new vulkan::RenderPass({tex, _depth_buffer});

	fb = new vulkan::FrameBuffer(_render_pass, {
		tex,
		_depth_buffer});
}

void HDRRendererVulkan::RenderIntoData::render_into(Renderer *r, const RenderParams& params) {
	if (!r)
		return;

	auto cb = params.command_buffer;

	cb->set_viewport(dynamicly_scaled_area(fb.get()));

	cb->begin_render_pass(_render_pass, fb.get());

	r->draw(params);

	cb->end_render_pass();
}


HDRRendererVulkan::HDRRendererVulkan(Camera *_cam, int width, int height) : PostProcessorStage("hdr") {
	cam = _cam;
	ch_post_blur = PerformanceMonitor::create_channel("blur", channel);
	ch_out = PerformanceMonitor::create_channel("out", channel);

	into = RenderIntoData(width, height);
	fb_main = into.fb.get();

	Array<vulkan::Texture*> blur_tex;
	Array<DepthBuffer*> blur_depth;
	int bloom_input_w = width/2;
	int bloom_input_h = height;
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		int bloom_w = (i == 0) ? (BLOOM_HEIGHT0 * width) / height : bloom_input_w / BLUR_SCALE;
		int bloom_h = (i == 0) ? BLOOM_HEIGHT0 : bloom_input_h / BLUR_SCALE;

		// 1. horizontal
		blur_tex.add(new vulkan::Texture(bloom_w, bloom_input_h, "rgba:f16"));
		blur_depth.add(new DepthBuffer(bloom_w, bloom_input_h, "d:f32", true));
		// 2. vertical
		blur_tex.add(new vulkan::Texture(bloom_w, bloom_h, "rgba:f16"));
		blur_depth.add(new DepthBuffer(bloom_w, bloom_h, "d:f32", true));

		blur_render_pass[i*2] = new vulkan::RenderPass({blur_tex[i*2], blur_depth[i*2]});
		blur_render_pass[i*2+1] = new vulkan::RenderPass({blur_tex[i*2+1], blur_depth[i*2+1]});
		// without clear, we get artifacts from dynamic resolution scaling
		bloom_input_w = bloom_w;
		bloom_input_h = bloom_h;
	}
	for (auto t: blur_tex)
		t->set_options("wrap=clamp");

	shader_blur = resource_manager->load_shader("forward/blur.shader");

	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		blur_pipeline[i] = new vulkan::GraphicsPipeline(shader_blur.get(), blur_render_pass[i*2], 0, "triangles", "3f,3f,2f");
		blur_pipeline[i]->set_z(false, false);
		blur_pipeline[i]->rebuild();
	}
	for (int i=0; i<MAX_BLOOM_LEVELS*2; i++) {
		blur_ubo[i] = new UniformBuffer(sizeof(UBOBlur));
		blur_dset[i] = pool->create_set(shader_blur.get());
	}
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		bloom_levels[i].fb_temp = new vulkan::FrameBuffer(blur_render_pass[i*2], {blur_tex[i*2], blur_depth[i]});
		bloom_levels[i].fb_out = new vulkan::FrameBuffer(blur_render_pass[i*2+1], {blur_tex[i*2+1], blur_depth[i]});
	}

	shader_out = resource_manager->load_shader("forward/hdr.shader");
	Array<Texture*> tex = {into.fb->attachments[0].get(), bloom_levels[0].fb_out->attachments[0].get(), bloom_levels[1].fb_out->attachments[0].get(), bloom_levels[2].fb_out->attachments[0].get(), bloom_levels[3].fb_out->attachments[0].get()};
	out = RenderOutData(shader_out.get(), tex);



	vb_2d = new VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);

	light_meter.init(resource_manager, fb_main, channel);
}

HDRRendererVulkan::~HDRRendererVulkan() = default;

void HDRRendererVulkan::prepare(const RenderParams& params) {
	auto cb = params.command_buffer;
	PerformanceMonitor::begin(ch_prepare);
	gpu_timestamp_begin(cb, ch_prepare);

	if (!cam)
		cam = cam_main;

	auto sub_params = params.with_target(into.fb.get());
	sub_params.render_pass = into._render_pass;

	for (auto c: children)
		c->prepare(sub_params);

	auto source = dynamicly_scaled_source();
	if (vb_2d_current_source != source) {
		vb_2d->create_quad(rect::ID_SYM, source);
		vb_2d_current_source = source;
	}

	into.render_into(children[0], sub_params);

	// render blur into fb_small2!
	PerformanceMonitor::begin(ch_post_blur);
	gpu_timestamp_begin(cb, ch_post_blur);
	auto bloom_input = into.fb.get();
	float threshold = 1.0f;
	for (int i=0; i<MAX_BLOOM_LEVELS; i++) {
		process_blur(cb, bloom_input, bloom_levels[i].fb_temp.get(), threshold, i*2);
		process_blur(cb, bloom_levels[i].fb_temp.get(), bloom_levels[i].fb_out.get(), 0.0f, i*2+1);
		bloom_input = bloom_levels[i].fb_out.get();
		threshold = 0;
	}
	gpu_timestamp_end(cb, ch_post_blur);
	PerformanceMonitor::end(ch_post_blur);

	light_meter.measure(cb, fb_main);
	if (cam->auto_exposure)
		light_meter.adjust_camera(cam);

	gpu_timestamp_end(cb, ch_prepare);
	PerformanceMonitor::end(ch_prepare);

}

void HDRRendererVulkan::draw(const RenderParams& params) {
	auto cb = params.command_buffer;


	PerformanceMonitor::begin(ch_out);
	gpu_timestamp_begin(cb, ch_out);
	out.render_out(cb, {cam->exposure, cam->bloom_factor, 2.2f, resolution_scale_x, resolution_scale_y}, cam->exposure, params);
	gpu_timestamp_end(cb, ch_out);
	PerformanceMonitor::end(ch_out);
}

void HDRRendererVulkan::process_blur(CommandBuffer *cb, FrameBuffer *source, FrameBuffer *target, float threshold, int iaxis) {
	const vec2 AXIS[2] = {{1,0}, {0,1}};
	//const float SCALE[2] = {(float)BLUR_SCALE, 1};
	UBOBlur u;
	u.radius = (iaxis <= 1) ? 5 : 11;//cam->bloom_radius * resolution_scale_x * 4 / (float)BLUR_SCALE;
	u.threshold = threshold / cam->exposure;
	u.axis = AXIS[iaxis % 2];
	blur_ubo[iaxis]->update(&u);
	blur_dset[iaxis]->set_buffer(0, blur_ubo[iaxis]);
	blur_dset[iaxis]->set_texture(1, source->attachments[0].get());
	blur_dset[iaxis]->update();

	auto rp = blur_render_pass[iaxis];

	cb->begin_render_pass(rp, target);
	cb->set_viewport(dynamicly_scaled_area(target));

	cb->bind_pipeline(blur_pipeline[iaxis / 2]);
	cb->bind_descriptor_set(0, blur_dset[iaxis]);

	cb->draw(vb_2d.get());

	cb->end_render_pass();

	//process(cb, {source->attachments[0].get()}, target, shader_blur.get());
}

void HDRRendererVulkan::LightMeter::init(ResourceManager* resource_manager, FrameBuffer* frame_buffer, int channel) {
	ch_post_brightness = PerformanceMonitor::create_channel("expo", channel);
	compute = new ComputeTask(resource_manager->load_shader("compute/brightness.shader"));
	params = new UniformBuffer(8);
	buf = new ShaderStorageBuffer(256*4);
	compute->bind_texture(0, frame_buffer->attachments[0].get());
	compute->bind_storage_buffer(1, buf);
	compute->bind_uniform_buffer(2, params);
}

void HDRRendererVulkan::LightMeter::measure(CommandBuffer* cb, FrameBuffer* frame_buffer) {
	PerformanceMonitor::begin(ch_post_brightness);
	gpu_timestamp_begin(cb, ch_post_brightness);

	int pp[2] = {frame_buffer->width, frame_buffer->height};
	params->update(&pp);

	int NBINS = 256;
	const int NSAMPLES = 256;
	if (histogram.num == NBINS) {
		void* p = buf->map();
		memcpy(&histogram[0], p, NBINS*sizeof(int));
		buf->unmap();
		//msg_write(str(histogram));

		int thresh = (NSAMPLES * 16 * 16) / 200 * 199;
		int n = 0;
		int ii = 0;
		for (int i=0; i<NBINS; i++) {
			n += histogram[i];
			if (n > thresh) {
				ii = i;
				break;
			}
		}
		brightness = pow(2.0f, ((float)ii / (float)NBINS) * 20.0f - 10.0f);
	}

	histogram.resize(NBINS);
	memset(&histogram[0], 0, NBINS * sizeof(int));
	buf->update(&histogram[0]);
	compute->dispatch(cb, NSAMPLES, 1, 1);

	gpu_timestamp_end(cb, ch_post_brightness);
	PerformanceMonitor::end(ch_post_brightness);
}

void HDRRendererVulkan::LightMeter::adjust_camera(Camera *cam) {
	float exposure = clamp((float)pow(1.0f / brightness, 0.8f), cam->auto_exposure_min, cam->auto_exposure_max);
	if (exposure > cam->exposure)
		cam->exposure *= 1.05f;
	if (exposure < cam->exposure)
		cam->exposure /= 1.05f;
}

#endif
