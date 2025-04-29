#include "TextureRenderer.h"

#ifdef USING_VULKAN
#include <helper/PerformanceMonitor.h>
#include <renderer/base.h>
#include "../../graphics-impl.h"

TextureRenderer::TextureRenderer(const string& name, const shared_array<Texture>& tex, const Array<string>& options) : RenderTask(name) {
	textures = tex;
	render_pass = new RenderPass(weak(textures), options);
	frame_buffer = new FrameBuffer(render_pass.get(), textures);
}

TextureRenderer::~TextureRenderer() = default;

void TextureRenderer::set_area(const rect& _area) {
	user_area = _area;
	override_area = true;
}


void TextureRenderer::prepare(const RenderParams &params) {
	Renderer::prepare(params);
}


void TextureRenderer::render(const RenderParams& params) {
	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);
	auto area = frame_buffer->area();
	if (override_area)
		area = user_area;

	auto p = params.with_target(frame_buffer.get()).with_area(area);
	p.render_pass = render_pass.get();

	auto cb = params.command_buffer;

	cb->begin_render_pass(render_pass.get(), frame_buffer.get());
	cb->set_viewport(area);
	cb->set_bind_point(vulkan::PipelineBindPoint::GRAPHICS);

	if (clear_z)
		cb->clear(area, clear_colors, clear_z);

	draw(p);
	cb->end_render_pass();
	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
}

#endif
