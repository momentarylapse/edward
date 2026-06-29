//
// Created by michi on 9/3/25.
//

#pragma once

#include "Node.h"

class Painter;

namespace gui {

class Canvas : public Node {
public:
	Canvas();
	~Canvas() override;

	//void _set_option(const string &k, const string &v);

	virtual void on_draw(Painter* p) {}
};

}