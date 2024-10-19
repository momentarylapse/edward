/*
 * TextureRendererGL.h
 *
 *  Created on: Nov 10, 2023
 *      Author: michi
 */

#pragma once


#include "../Renderer.h"
#ifdef USING_OPENGL

class TextureRendererGL : public Renderer {
public:
	explicit TextureRendererGL(FrameBuffer *fb);

	void render(float aspect_ratio);

	FrameBuffer *fb;
};

#endif
