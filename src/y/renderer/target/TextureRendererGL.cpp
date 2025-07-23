/*
 * TextureRendererGL.cpp
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#include "TextureRenderer.h"
#ifdef USING_OPENGL
#include <lib/profiler/Profiler.h>
#include <renderer/base.h>
#include <graphics-impl.h>

#include "lib/os/msg.h"

TextureRenderer::TextureRenderer(const string& name, const shared_array<Texture>& textures, const Array<string>& options) : RenderTask(name) {
	frame_buffer = new FrameBuffer(textures);
}

TextureRenderer::~TextureRenderer() = default;

RenderParams TextureRenderer::make_params(const RenderParams &params) const {
	auto area = frame_buffer->area();
	if (override_area)
		area = user_area;

	auto p = params.with_target(frame_buffer.get()).with_area(area);
	return p;
}

void TextureRenderer::set_area(const rect& _area) {
	user_area = _area;
	override_area = true;
}

void TextureRenderer::set_layer(int layer) {
	try {
		frame_buffer->update_x(textures, layer);
	} catch(Exception &e) {
		msg_error(e.message());
	}
}

void TextureRenderer::render(const RenderParams& params) {
	profiler::begin(channel);
	gpu_timestamp_begin(params, channel);

	auto p = make_params(params);

	nix::bind_frame_buffer(frame_buffer.get());
	nix::set_viewport(p.area);
	if (clear_z)
		nix::clear_z();
	for (int i=0; i<clear_colors.num; i++)
		frame_buffer->clear_color(i, clear_colors[i]);

	prepare_children(p);

	for (auto c: children)
		c->draw(p);

	gpu_timestamp_end(params, channel);
	profiler::end(channel);
}


#endif
