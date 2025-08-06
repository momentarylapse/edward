/*
 * GuiRendererGL.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>
#ifdef USING_OPENGL

class GuiRendererGL : public yrenderer::Renderer {
public:
	explicit GuiRendererGL(yrenderer::Context* ctx);

	void draw(const yrenderer::RenderParams& params) override;

	void draw_gui(const yrenderer::RenderParams& params, ygfx::FrameBuffer *source);

	shared<ygfx::Shader> shader;
	owned<ygfx::VertexBuffer> vb;
};

#endif
