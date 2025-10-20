/*
 * Picture.cpp
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#include "Picture.h"
#include <lib/math/rect.h>
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <EngineData.h>



namespace gui {

Picture::Picture() : Picture(rect::ID, engine.context ? engine.context->tex_white : nullptr) {}

Picture::Picture(const rect &r, shared<ygfx::Texture> tex, const rect &s) :
	Node(r)
{
	type = Type::PICTURE;
	source = s;
	texture = tex;
	bg_blur = 0;
	angle = 0;
	visible = true;
}

Picture::~Picture() = default;

void Picture::_set_option(const string &k, const string &v) {
	if (k == "texture") {
		// ...
	} else if (k == "angle") {
		angle = v._float();
	} else {
		Node::_set_option(k, v);
	}
}



}
