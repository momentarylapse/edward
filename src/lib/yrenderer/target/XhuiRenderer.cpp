//
// Created by Michael Ankele on 2025-01-28.
//

#include "XhuiRenderer.h"
#if __has_include(<lib/xhui/Painter.h>)
#include <lib/xhui/Painter.h>
#include <lib/xhui/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>


#ifdef USING_OPENGL
namespace nix {
	extern FrameBuffer* cur_framebuffer;
}
#endif

namespace yrenderer {

RenderParams XhuiRenderer::extract_params(Painter* p) {
	auto pp = (xhui::Painter*)p;
	RenderParams params;
	params.area = pp->native_area;
#ifdef USING_VULKAN
	params.command_buffer = pp->cb;
	params.render_pass = pp->context->render_pass;
	params.frame_buffer = pp->context->current_frame_buffer();
#else
	params.frame_buffer = pp->context->context->ctx->default_framebuffer;
#endif
	params.target_is_window = true;
	params.desired_aspect_ratio = pp->native_area.width() / pp->native_area.height();
	native_area_window = pp->native_area_window;
	return params;
}

void XhuiRenderer::before_draw(Painter* p) {
	const auto params = extract_params(p);

	for (auto c: children)
		c->prepare(params);
}

void XhuiRenderer::render(const RenderParams& params) {
#ifdef USING_VULKAN
	params.command_buffer->set_viewport(params.area);
#endif
#ifdef USING_OPENGL
	//nix::set_viewport(params.area);
	// OpenGL' coordinate system is annoying (-_-)'
	nix::set_viewport(params.area + vec2(0, native_area_window.y1 - params.area.y1));
	nix::set_scissor(params.area, params.frame_buffer->area());
#endif

	for (auto c: children)
		c->draw(params);
}

void XhuiRenderer::draw(Painter* p) {
	const auto clip0 = p->clip();
	const auto params = extract_params(p);

	render(params);

	reinterpret_cast<xhui::Painter*>(p)->prepare_2d_drawing();
	p->set_clip(clip0);
}

}
#else

namespace yrenderer {
	RenderParams XhuiRenderer::extract_params(Painter*) {
		return {};
	}
	void XhuiRenderer::before_draw(Painter*) {}
	void XhuiRenderer::render(const RenderParams&) {}
	void XhuiRenderer::draw(Painter*) {}
}


#endif
