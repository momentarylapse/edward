//
// Created by Michael Ankele on 2025-01-28.
//

#include "XhuiRenderer.h"
#if __has_include(<lib/xhui/Painter.h>)
#include <lib/xhui/Painter.h>
#include <lib/xhui/Context.h>


void XhuiRenderer::render(const RenderParams& params) {
#ifdef USING_VULKAN
	params.command_buffer->set_viewport(params.area);
#endif
	for (auto c: children)
		c->prepare(params);
	for (auto c: children)
		c->draw(params);
#ifdef USING_VULKAN
	params.command_buffer->set_viewport(native_area_window);
#endif
}

void XhuiRenderer::render(Painter* p) {
	auto pp = (xhui::Painter*)p;
	RenderParams params;
	params.area = pp->native_area;
#ifdef USING_VULKAN
	params.command_buffer = pp->cb;
	params.render_pass = pp->context->render_pass;
	params.frame_buffer = pp->context->current_frame_buffer();
#endif
	params.desired_aspect_ratio = pp->native_area.width() / pp->native_area.height();
	native_area_window = pp->native_area_window;
	render(params);
}
#endif
