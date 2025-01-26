//
// Created by Michael Ankele on 2025-01-26.
//

#include "Dialog.h"

#include <lib/os/msg.h>

#include "Window.h"
#include "Theme.h"
#include "Painter.h"

namespace xhui {

Array<Dialog*> _dialogs_;

Dialog::Dialog(const string& _title, int _width, int _height, Panel* parent) : Panel("dialog") {
	title = _title;
	width = _width;
	height = _height;
	owner = parent;
	owner->controls.add(this);
	window = parent->window;
	window->dialog = this;
	_area = {0, (float)width, 0, (float)height};
	padding = Theme::_default.window_margin;

	_dialogs_.add(this);
	window->request_redraw();
}

Dialog::~Dialog() {
	for (int i=0; i<_dialogs_.num; i++)
		if (_dialogs_[i] == this)
			_dialogs_.erase(i);

	window->dialog = nullptr;
	for (int i=0; i<owner->controls.num; i++)
		if (owner->controls[i] == this) {
			owner->controls.erase(i);
		}
	window->hover_control = nullptr;
}


void Dialog::negotiate_area(const rect& available) {
	_area = available;
	if (top_control)
		top_control->negotiate_area(smaller_rect(available, padding));
}

void Dialog::_draw(Painter* p) {
	p->set_color(Theme::_default.background);
	p->set_roundness(Theme::_default.window_radius);
	p->draw_rect({_area.p00() - vec2(0, 30), _area.p11()});
	p->set_roundness(0);
	p->set_color(Theme::_default.text_disabled);
	p->set_font_size(Theme::_default.font_size * 1.3f);
	p->draw_str(_area.p00() + vec2(20, -20), title);
	if (top_control)
		top_control->_draw(p);
}

void Dialog::request_destroy() {
	_destroy_requested = true;
}





} // xhui