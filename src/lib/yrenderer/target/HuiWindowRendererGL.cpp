#if __has_include(<lib/hui/hui.h>)
#include "HuiWindowRenderer.h"
#ifdef USING_OPENGL
#include <lib/ygraphics/graphics-impl.h>
#include <lib/hui/hui.h>

namespace yrenderer {

HuiWindowRenderer::HuiWindowRenderer(Context* _ctx) : Renderer("hui") {
	ctx = _ctx;
}

void HuiWindowRenderer::render_frame() {
	auto e = hui::get_event();
	int width = e->column;
	int height = e->row;

	// save the default frame buffer etc!
	nix::start_frame_hui(ctx);

	auto params = RenderParams::into_window(ctx->default_framebuffer);

	prepare(params);

	//nix::start_frame_hui(ctx);
	nix::bind_frame_buffer(params.frame_buffer);
	nix::set_viewport(params.area);

	nix::set_viewport(params.area);

	draw(params);

	nix::end_frame_hui();
}

}

#endif
#endif
