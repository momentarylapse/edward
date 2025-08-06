//
// Created by Michael Ankele on 2025-01-26.
//

#include "Dialog.h"

#include <lib/os/msg.h>

#include "Window.h"
#include "Theme.h"
#include "Painter.h"
#include "controls/Button.h"
#include "controls/Grid.h"
#include "controls/Label.h"

namespace xhui {

static constexpr float HEADER_HEIGHT = 30;

DialogFlags operator|(DialogFlags a, DialogFlags b) {
	return (DialogFlags)((int)a | (int)b);
}

bool operator&(DialogFlags a, DialogFlags b) {
	return (int)a & (int)b;
}

class DialogHeader : public Grid {
public:
	DialogHeader(const string& id, const string& title, const std::function<void()>& f) : Grid(id) {
		label = new Label(id + ":label", title);
		label->bold = true;
		label->align = Label::Align::Center;
		label->size_mode_x = SizeMode::Expand;
		close_button = new CallbackButton(id + ":close", "x", f);
		close_button->size_mode_x = SizeMode::Shrink;
		Grid::add_child(label, 0, 0);
		Grid::add_child(close_button, 1, 0);
	}
	void set_title(const string& title) {
		label->set_string(title);
	}
	Label* label;
	CallbackButton* close_button;
};

class DialogOutside : public Control {
public:
	explicit DialogOutside(const std::function<void()>& f) : Control(":outside:") {
		f_click = f;
	}
	void on_left_button_down(const vec2& m) override {
		f_click();
	}
	std::function<void()> f_click;
};

Dialog::Dialog(const string& _title, int _width, int _height, Panel* parent, DialogFlags _flags) : Panel("dialog") {
	type = ControlType::Dialog;
	flags = _flags;
	outside = new DialogOutside([this] {
		if (flags & DialogFlags::CloseByClickOutside)
			request_destroy();
	});
	if (!(flags & DialogFlags::NoHeader))
		header = new DialogHeader(id + ":header", _title, [this] {
			if (!handle_event(event_id::Close, event_id::Close, true))
				request_destroy();
		});
	width = _width;
	height = _height;
	pos = {0, 0};
	_area = {0, (float)width, 0, (float)height};
	padding = Theme::_default.window_margin;

	// forward default activation ([Return] key) to default button
	event_x("*", event_id::ActivateDialogDefault, [this] {
		for (auto c: controls)
			if (c->type == ControlType::Button) {
				auto b = static_cast<Button*>(c);
				if (b->_default)
					b->emit_event(event_id::Activate, true);
			}
	});
}

Dialog::Dialog(const string& id, Panel* parent) : Dialog("", 400, 300, parent) {
	from_resource(id);
}

Dialog::~Dialog() = default;

Array<Control*> Dialog::get_children(ChildFilter f) const {
	Array<Control*> r;
	r.add(outside.get());
	if (header)
		r.add(header.get());
	if (top_control)
		r.add(top_control.get());
	return r;
}



void Dialog::negotiate_area(const rect& available) {
	outside->_area = {0, 10000, 0, 10000}; // what could go wrong :P
	if (header) {
		_area = {available.p00() - vec2(0, HEADER_HEIGHT), available.p11()};
		header->negotiate_area({_area.p00(), available.p10()});
	} else {
		_area = {available.p00(), available.p11()};
	}
	if (top_control)
		top_control->negotiate_area(available.grow(- padding));
}

void Dialog::on_key_down(int key) {
	if (flags & DialogFlags::CloseByEscape and key == KEY_ESCAPE)
		request_destroy();
}


void Dialog::_draw(Painter* p) {
	p->set_color(Black.with_alpha(0.3f));
	p->set_roundness(25);
	p->softness = 50;
	p->draw_rect(_area.grow(30));
	p->softness = 0;

	// background
	p->set_color(Theme::_default.background);
	p->set_roundness(Theme::_default.window_radius);
	p->draw_rect(_area);
	p->set_roundness(0);

	if (header)
		header->_draw(p);
	if (top_control)
		top_control->_draw(p);
}

void Dialog::request_destroy() {
	_destroy_requested = true;
}

void Dialog::set_title(const string& title) {
	header->set_title(title);
}

rect Dialog::suggest_area(const rect& parent_area) const {
	const vec2 m = parent_area.center();
	const vec2 size = vec2((float)width, (float)height);
	if (flags & DialogFlags::FixedPosition)
		return {pos, pos + size};
	return {m - size/2, m + size/2};
}



Dialog* as_dialog(Control* c) {
	if (c->type == ControlType::Dialog)
		return static_cast<Dialog*>(c);
	return nullptr;
}



} // xhui