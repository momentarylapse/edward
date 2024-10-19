/*
 * TextureRendererGL.cpp
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#include "TextureRendererGL.h"
#ifdef USING_OPENGL
#include <graphics-impl.h>
//#include "../../helper/PerformanceMonitor.h"

TextureRendererGL::TextureRendererGL(FrameBuffer *_fb) : Renderer("tex") {
	fb = _fb;
}

void TextureRendererGL::render(float aspect_ratio) {
	nix::bind_frame_buffer(fb);

	draw(RenderParams::into_texture(fb, 1.0f));
}
#endif
