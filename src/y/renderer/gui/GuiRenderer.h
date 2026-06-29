/*
 * GuiRenderer.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>

namespace gui {
	class Node;
}

namespace ygfx {
	struct DrawingHelperData;
	class Painter;
}

class GuiRenderer : public yrenderer::Renderer {
public:
	explicit GuiRenderer(yrenderer::Context* ctx);
	~GuiRenderer() override;

	void prepare(const yrenderer::RenderParams &params) override;
	void draw(const yrenderer::RenderParams& params) override;
	void draw_node(ygfx::Painter& p, gui::Node* n, const color& group_col);

	owned<ygfx::DrawingHelperData> aux;
};

