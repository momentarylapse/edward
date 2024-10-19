/*
 * GuiRendererGL.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include "../Renderer.h"
#ifdef USING_OPENGL

class GuiRendererGL : public Renderer {
public:
	GuiRendererGL();

	void draw(const RenderParams& params) override;

	void draw_gui(FrameBuffer *source);

	shared<Shader> shader;
	owned<VertexBuffer> vb;
};

#endif
