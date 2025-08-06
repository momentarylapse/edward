//
// Created by Michael Ankele on 2025-01-28.
//

#include "XhuiRenderer.h"
#if __has_include(<lib/xhui/Painter.h>)
#include <lib/xhui/Painter.h>
#include <lib/xhui/Context.h>
#include <lib/ygraphics/graphics-impl.h>

namespace yrenderer {

RenderParams XhuiRenderer::extract_params(Painter* p) {
	auto pp = (xhui::Painter*)p;
	RenderParams params;
	params.area = pp->native_area;
#ifdef USING_VULKAN
	params.command_buffer = pp->cb;
	params.render_pass = pp->context->render_pass;
	params.frame_buffer = pp->context->current_frame_buffer();
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
	nix::set_viewport(params.area);
	nix::set_scissor(params.area);
#endif

	for (auto c: children)
		c->draw(params);

#ifdef USING_OPENGL
	nix::set_scissor(rect::EMPTY);
	nix::set_viewport(native_area_window);
#endif
#ifdef USING_VULKAN
	params.command_buffer->set_viewport(native_area_window);
#endif
}

void XhuiRenderer::draw(Painter* p) {
	const auto params = extract_params(p);

	render(params);
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
