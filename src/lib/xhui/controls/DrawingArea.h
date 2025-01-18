#pragma once

#include "Control.h"

namespace xhui {



class DrawingArea : public Control {
public:
	DrawingArea(const string &id);

	void _draw(Painter *p) override;
};

}