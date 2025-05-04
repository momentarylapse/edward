//
// Created by Michael Ankele on 2025-01-28.
//

#ifndef XHUIRENDERER_H
#define XHUIRENDERER_H

#include "../Renderer.h"

class Painter;

class XhuiRenderer : public RenderTask {
public:
	rect native_area_window = rect::ID;
	XhuiRenderer() : RenderTask("xhui") {
	}
	RenderParams extract_params(Painter* p);
	void render(const RenderParams& params) override;

	// call in event_xp(window->id, xhui::event_id::JustBeforeDraw, [this] (Painter* p) { ... });
	void prepare(Painter* p);
	// call in event_xp("area", xhui::event_id::Draw, [this] (Painter* p) { ... });
	void render(Painter* p);
};


#endif //XHUIRENDERER_H
