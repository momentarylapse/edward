/*
 * HDRRendererGL.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include "PostProcessor.h"
#ifdef USING_OPENGL

class vec2;
class Camera;
class ComputeTask;

class HDRRendererGL : public PostProcessorStage {
public:
	HDRRendererGL(Camera *cam, int width, int height);
	~HDRRendererGL() override;

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	void process_blur(FrameBuffer *source, FrameBuffer *target, float r, float threshold, const vec2 &axis);
	void process(const Array<Texture*> &source, FrameBuffer *target, Shader *shader);
	void render_out(FrameBuffer *source, Texture *bloom, const RenderParams& params);

	Camera *cam;

	shared<FrameBuffer> fb_main;
	shared<FrameBuffer> fb_main_ms;

	static const int MAX_BLOOM_LEVELS = 4;

	struct BloomLevel {
		shared<FrameBuffer> fb_temp;
		shared<FrameBuffer> fb_out;
	} bloom_levels[MAX_BLOOM_LEVELS];

	DepthBuffer *_depth_buffer = nullptr;
	shared<Shader> shader_blur;
	shared<Shader> shader_out;
	shared<Shader> shader_resolve_multisample;

	owned<VertexBuffer> vb_2d;

	int ch_post_blur = -1, ch_out = -1;

	struct LightMeter {
		void init(ResourceManager* resource_manager, FrameBuffer* frame_buffer, int channel);
		ComputeTask* compute;
		UniformBuffer* params;
		ShaderStorageBuffer* buf;
		Array<int> histogram;
		float brightness;
		int ch_post_brightness = -1;
		void measure(FrameBuffer* frame_buffer);
		void adjust_camera(Camera* cam);
	} light_meter;
};

#endif
