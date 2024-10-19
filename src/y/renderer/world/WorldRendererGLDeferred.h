/*
 * WorldRendererGLDeferred.h
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#pragma once

#include "WorldRendererGL.h"
#ifdef USING_OPENGL

class WorldRendererGLDeferred : public WorldRendererGL {
public:

	shared<FrameBuffer> gbuffer;
	shared<Shader> shader_gbuffer_out;
	UniformBuffer *ssao_sample_buffer;
	int ch_gbuf_out = -1;
	int ch_trans = -1;

	owned<GeometryRendererGL> geo_renderer_trans;

	WorldRendererGLDeferred(Camera *cam, int width, int height);
	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	//void render_into_texture(FrameBuffer *fb, Camera *cam) override;
	void render_into_gbuffer(FrameBuffer *fb, const RenderParams& params);
	void draw_background(FrameBuffer *fb, const RenderParams& params);


	void render_out_from_gbuffer(FrameBuffer *source, const RenderParams& params);
};

#endif
