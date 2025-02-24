#pragma once

#include "CheckBox.h"

namespace xhui {

class RadioButton : public CheckBox {
public:
	RadioButton(const string &id, const string &title);

	void check(bool checked) override;

	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;

	void _draw(Painter *p) override;

	Array<CheckBox*> group() const;
};

}
