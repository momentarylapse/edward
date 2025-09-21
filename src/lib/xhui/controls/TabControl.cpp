//
// Created by michi on 09.02.25.
//

#include "TabControl.h"
#include "ToggleButton.h"
#include "Grid.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../../base/iter.h"

namespace xhui {

class TabControlHeader : public Grid {
public:
	explicit TabControlHeader(const string& id, const Array<string>& headers, const std::function<void(int)>& f) : Grid(id) {
		callback = f;
		spacing = 2;
		for (const auto&& [i, h] : enumerate(headers)) {
			auto b = new CallbackToggleButton(id + i2s(i), h, [this, i=i] {
				current_page = i;
				update_buttons();
				callback(i);
			});
			b->padding.x1 = b->padding.x2 = Theme::_default.button_margin_y;
			b->size_mode_x = SizeMode::Shrink;
			b->size_mode_y = SizeMode::Shrink;
			b->flat = true;
			Grid::add_child(b, i, 0);
			buttons.add(b);
		}
		current_page = 0;
		update_buttons();
	}
	void set_current_page(int p) {
		current_page = p;
		update_buttons();
	}
	void update_buttons() {
		for (auto&& [i, b]: enumerate(buttons))
			b->check(i == current_page);
	}
	int current_page;
	std::function<void(int)> callback;
	Array<CallbackToggleButton*> buttons;
};

TabControl::TabControl(const string& id, const string& title) : Control(id) {
	header = new TabControlHeader(id + ":header", title.explode("\\"), [this](int i) {
		current_page = i;
		emit_event(event_id::Changed, true);
	});

	for (const string& h: title.explode("\\"))
		pages.add({h, nullptr});
	current_page = 0;
}

vec2 TabControl::get_content_min_size() const {
	vec2 s = {0, 0};

	for (const auto& p: pages)
		if (p.child)
			s = vec2::max(s, p.child->get_effective_min_size());

	if (show_header) {
		const vec2 cs = header->get_effective_min_size();
		s.y += cs.y + Theme::_default.spacing;
	}
	return s;
}

void TabControl::negotiate_area(const rect& available) {
	_area = available;
	vec2 p00 = available.p00();
	if (show_header) {
		const vec2 s = header->get_effective_min_size();
		header->negotiate_area({available.p00(), available.p10() + vec2(0, s.y)});
		p00 = header->_area.p01() + vec2(0, Theme::_default.spacing);
	}
	for (auto& p: pages)
		if (p.child)
			p.child->negotiate_area({p00, available.p11()});
}

vec2 TabControl::get_greed_factor() const {
	vec2 f = {0, 0};
	for (const auto& p: pages)
		if (p.child)
			f = vec2::max(f, p.child->get_greed_factor());
	return f;
}

Array<Control*> TabControl::get_children(ChildFilter f) const {
	Array<Control*> children;
	if (f == ChildFilter::All or show_header)
		children.add(header.get());
	for (auto&& [i, p]: enumerate(pages))
		if (p.child and (f == ChildFilter::All or i == current_page))
			children.add(p.child.get());
	return children;
}

void TabControl::add_child(shared<Control> c, int x, int y) {
	if (x >= 0 and x < pages.num) {
		if (pages[x].child)
			pages[x].child->_unregister();
		pages[x].child = c;
		if (owner)
			c->_register(owner);
	} else if (x == pages.num) {
		pages.add({"+", c});
		if (owner)
			c->_register(owner);
	}
}

int TabControl::get_int() {
	return current_page;
}

void TabControl::set_int(int i) {
	if (i < 0 or i >= pages.num)
		return;
	current_page = i;
	header->set_current_page(i);
	request_redraw();
}

void TabControl::_draw(Painter* p) {
	if (show_header)
		header->_draw(p);

	if (pages[current_page].child)
		pages[current_page].child->_draw(p);
}

	void TabControl::set_option(const string& key, const string& value) {
	if (key == "bar") {
		show_header = value._bool();
		request_redraw();
	} else if (key == "nobar") {
		show_header = false;
		request_redraw();
	} else {
		Control::set_option(key, value);
	}
}







} // xhui