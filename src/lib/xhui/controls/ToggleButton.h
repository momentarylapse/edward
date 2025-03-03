//
// Created by Michael Ankele on 2025-03-03.
//

#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include "Button.h"

namespace xhui {

class ToggleButton : public Button {
public:
	ToggleButton(const string &id, const string &title);

	bool is_checked() override;
	void check(bool checked) override;
	void on_click() override;
	void _draw(Painter* p) override;

	bool checked;
};


class CallbackToggleButton : public ToggleButton {
public:
	CallbackToggleButton(const string& id, const string& title, const std::function<void()>& f) : ToggleButton(id, title) {
		callback = f;
	}
	void on_click() override {
		ToggleButton::on_click();
		callback();
	}
	std::function<void()> callback;
};

} // xhui

#endif //TOGGLEBUTTON_H
