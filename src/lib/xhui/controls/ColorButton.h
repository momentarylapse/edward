//
// Created by Michael Ankele on 2025-02-03.
//

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include "Button.h"
#include "../../image/color.h"

namespace xhui {

class ColorButton : public Button {
public:
	explicit ColorButton(const string& id);
	void _draw(Painter* p) override;

	void on_click() override;

	color _color;
	void set_color(const color& c) override;
	color get_color() override;
};

} // xhui

#endif //COLORBUTTON_H
