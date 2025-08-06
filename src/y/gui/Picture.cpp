/*
 * Picture.cpp
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#include "Picture.h"
#include "../lib/math/rect.h"
#include "../lib/math/vec3.h"
#include "../lib/math/mat4.h"
#include <lib/ygraphics/graphics-impl.h>
#include <iostream>


/*vulkan::Shader *Picture::shader = nullptr;
vulkan::Pipeline *Picture::pipeline = nullptr;
vulkan::VertexBuffer *Picture::vertex_buffer = nullptr;
vulkan::RenderPass *Picture::render_pass = nullptr;*/


namespace gui {

Picture::Picture(const rect &r, shared<ygfx::Texture> tex, const rect &s, shared<ygfx::Shader> _shader) :
	Node(r)
{
	type = Type::PICTURE;
	source = s;
	texture = tex;
	bg_blur = 0;
	angle = 0;
	visible = true;
	shader = _shader;
}

Picture::Picture(const rect &r, shared<ygfx::Texture> tex, const rect &s) : Picture(r, tex, s, nullptr) {
}

Picture::~Picture() = default;

void Picture::__init__(const rect &r, shared<ygfx::Texture> tex, const rect &s) {
	new(this) Picture(r, tex, s);
	source = s;
}

void Picture::__delete__() {
	this->Picture::~Picture();
}


}
