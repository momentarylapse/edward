#pragma once

#include "Control.h"

namespace xhui {



class DrawingArea : public Control {
public:
	explicit DrawingArea(const string &id);

	void _draw(Painter *p) override;

	bool first_draw = true;
};

}