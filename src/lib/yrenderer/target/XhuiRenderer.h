//
// Created by Michael Ankele on 2025-01-28.
//

#ifndef XHUIRENDERER_H
#define XHUIRENDERER_H

#include <lib/yrenderer/Renderer.h>

class Painter;

namespace yrenderer {

class XhuiRenderer : public RenderTask {
public:
	rect native_area_window = rect::ID;
	explicit XhuiRenderer(Context* ctx) : RenderTask(ctx, "xhui") {}
	RenderParams extract_params(Painter* p);
	void render(const RenderParams& params) override;

	// call in event_xp(window->id, xhui::event_id::JustBeforeDraw, [this] (Painter* p) { ... });
	void before_draw(Painter* p);
	// call in event_xp("area", xhui::event_id::Draw, [this] (Painter* p) { ... });
	void draw(Painter* p);
};

}


#endif //XHUIRENDERER_H
