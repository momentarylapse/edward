/*
 * TextureRendererGL.cpp
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#include "TextureRenderer.h"
#ifdef USING_OPENGL
#include <helper/PerformanceMonitor.h>
#include <renderer/base.h>
#include <graphics-impl.h>

TextureRenderer::TextureRenderer(const string& name, const shared_array<Texture>& textures, const Array<string>& options) : RenderTask(name) {
	frame_buffer = new FrameBuffer(textures);
}

TextureRenderer::~TextureRenderer() = default;

void TextureRenderer::set_area(const rect& _area) {
	user_area = _area;
	override_area = true;
}

void TextureRenderer::render(const RenderParams& params) {
	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);
	nix::bind_frame_buffer(frame_buffer.get());

	auto area = frame_buffer->area();
	if (override_area)
		area = user_area;

	nix::set_viewport(area);
	if (clear_z)
		nix::clear_z();
	for (int i=0; i<clear_colors.num; i++)
		frame_buffer->clear_color(i, clear_colors[i]);
	draw(RenderParams::into_texture(frame_buffer.get(), params.desired_aspect_ratio).with_area(area));
	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
}

void TextureRenderer::prepare(const RenderParams& params) {
	Renderer::prepare(params);
}



#endif
