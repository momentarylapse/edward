//
// Created by michi on 9/3/25.
//

#include "Canvas.h"

namespace gui {
	Canvas::Canvas() : Canvas(rect::ID) {}

	Canvas::Canvas(const rect &r) :
		Node(r)
	{
		type = Type::CANVAS;
		visible = true;
	}

	Canvas::~Canvas() = default;
}