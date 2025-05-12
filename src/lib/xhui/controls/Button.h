#pragma once

#include "Label.h"
#include <functional>

namespace xhui {

class Button : public Control {
public:
	Button(const string &id, const string &title);

	vec2 get_content_min_size() const override;
	void negotiate_area(const rect& available) override;
	Array<Control*> get_children(ChildFilter f) const override;
	void set_string(const string& s) override;
	string get_string() override;
	void enable(bool enabled) override;

	void on_mouse_enter(const vec2& m) override;
	void on_mouse_leave(const vec2& m) override;
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;

	void _draw(Painter *p) override;
	void set_option(const string& key, const string& value) override;

	virtual void on_click();

	bool _default = false;
	bool primary = false;
	bool flat = false;
	rect padding;

	enum class State {
		DEFAULT,
		HOVER,
		PRESSED
	};
	State state;
	Label label;
};


class CallbackButton : public Button {
public:
	CallbackButton(const string& id, const string& title, const std::function<void()>& f) : Button(id, title) {
		callback = f;
	}
	void on_click() override {
		callback();
	}
	std::function<void()> callback;
};

}
