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

#include "helper/ResourceManager.h"


namespace gui {

Picture::Picture() {
	type = Type::PICTURE;
	source = rect::ID;
	bg_blur = 0;
	angle = 0;
	radius = 0;
	softness = 0;
	visible = true;
	allow_hover = true;
	size_mode_x = layout::SizeMode::Shrink;
	size_mode_y = layout::SizeMode::Shrink;
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

void Picture::set_option(const string &k, const string &v) {
	if (k == "texture") {
		texture = engine.resource_manager->load_texture(v);
	} else if (k == "angle") {
		angle = v._float();
	} else if (k == "radius") {
		radius = v._float();
	} else if (k == "softness") {
		softness = v._float();
	} else if (k == "source") {
		const auto a = Any::parse(v);
		if (a.is_list() and a.length() >= 4) {
			source.x1 = a[0].to_f32();
			source.x2 = a[1].to_f32();
			source.y1 = a[2].to_f32();
			source.y2 = a[3].to_f32();
		}
	} else {
		Node::set_option(k, v);
	}
}



}
