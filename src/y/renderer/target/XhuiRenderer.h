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
	void render(const RenderParams& params) override;
	void render(Painter* p);
};


#endif //XHUIRENDERER_H
