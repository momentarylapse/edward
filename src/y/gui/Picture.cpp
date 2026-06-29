/*
 * Picture.cpp
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#include "Picture.h"
#include <lib/layout/Node.h>
#include <lib/math/rect.h>
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <EngineData.h>


namespace gui {

Picture::Picture() {
	type = Type::PICTURE;
	source = rect::ID;
	bg_blur = 0;
	angle = 0;
	radius = 0;
	visible = true;
	allow_hover = true;
}

Picture::Picture(const rect &r, shared<ygfx::Texture> tex, const rect &s) : Picture() {
	size_mode_x = layout::SizeMode::Shrink;
	size_mode_y = layout::SizeMode::Shrink;
	min_width_user = r.width();
	min_height_user = r.height();
	margin.x1 = r.x1;
	margin.y1 = r.y1;
	source = s;
	texture = tex;
}

Picture::~Picture() = default;

vec2 Picture::get_content_min_size() const {
	return {0, 0};
}

void Picture::set_option(const string &k, const string &v) {
	if (k == "texture") {
		// ...
	} else if (k == "angle") {
		angle = v._float();
	} else if (k == "radius") {
		radius = v._float();
	} else {
		Node::set_option(k, v);
	}
}



}
