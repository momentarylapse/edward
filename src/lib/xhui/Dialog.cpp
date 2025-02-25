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

Dialog::Dialog(const string& _title, int _width, int _height, Panel* parent) : Panel("dialog") {
	header = new DialogHeader(id + ":header", _title, [this] {
		if (!handle_event(event_id::Close, event_id::Close, true))
			request_destroy();
	});
	width = _width;
	height = _height;
	_area = {0, (float)width, 0, (float)height};
	padding = Theme::_default.window_margin;
}

Dialog::Dialog(const string& id, Panel* parent) : Dialog("", 400, 300, parent) {
	from_resource(id);
}

Dialog::~Dialog() = default;

Array<Control*> Dialog::get_children(ChildFilter f) const {
	if (top_control)
		return {header.get(), top_control.get()};
	return {header.get()};
}



void Dialog::negotiate_area(const rect& available) {
	_area = {available.p00() - vec2(0, HEADER_HEIGHT), available.p11()};
	header->negotiate_area({_area.p00(), available.p10()});
	if (top_control)
		top_control->negotiate_area(smaller_rect(available, padding));
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






} // xhui