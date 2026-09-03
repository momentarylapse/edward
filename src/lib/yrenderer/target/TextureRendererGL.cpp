/*
 * TextureRendererGL.cpp
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#include "TextureRenderer.h"
#ifdef USING_OPENGL
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>

namespace yrenderer {

TextureRenderer::TextureRenderer(Context* ctx, const string& name, const shared_array<ygfx::Texture>& textures, const Array<string>& options) : RenderTask(ctx, name) {
	set_textures(textures);
}

TextureRenderer::~TextureRenderer() = default;

RenderParams TextureRenderer::make_params(const RenderParams &params) const {
	auto area = frame_buffer->area();
	if (override_area)
		area = user_area;

	auto p = params.with_target(frame_buffer.get()).with_area(area);
	return p;
}

void TextureRenderer::set_textures(const shared_array<ygfx::Texture>& tex) {
	textures = tex;
	frame_buffer = new ygfx::FrameBuffer(textures);
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
	ctx->gpu_timestamp_begin(params, channel);

	auto p = make_params(params);

	prepare_children(p);

	nix::bind_frame_buffer(frame_buffer.get());
	nix::set_viewport(p.area);
	rect aa = {p.area.x1, p.area.y1, frame_buffer->area().y2 - p.area.height(), frame_buffer->area().y2};
	nix::set_scissor(aa, frame_buffer->area());
	if (clear_z)
		frame_buffer->clear_depth(1.0);
	for (int i=0; i<clear_colors.num; i++)
		frame_buffer->clear_color(i, clear_colors[i]);
	nix::set_z(true, true);

	for (auto c: children)
		c->draw(p);

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

}


#endif
