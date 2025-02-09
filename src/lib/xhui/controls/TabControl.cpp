//
// Created by michi on 09.02.25.
//

#include "TabControl.h"
#include "Button.h"
#include "Grid.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../../base/iter.h"

namespace xhui {

class TabControlHeader : public Grid {
public:
	class HeaderButton : public Button {
		public:
		HeaderButton(const string& id, const string& title, std::function<void()> f) : Button(id, title) {
			callback = f;
			size_mode_x = SizeMode::Shrink;
			size_mode_y = SizeMode::Shrink;
		}
		void on_click() override {
			callback();
		}
		std::function<void()> callback;
	};

	explicit TabControlHeader(const string& id, const Array<string>& headers, std::function<void(int)> f) : Grid(id) {
		callback = f;
		for (const auto&& [i, h] : enumerate(headers)) {
			auto b = new HeaderButton(id + i2s(i), h, [this, i=i] {
				current_page = i;
				update_buttons();
				callback(i);
			});
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
			b->primary = (i == current_page);
		request_redraw();
	}
	int current_page;
	std::function<void(int)> callback;
	Array<HeaderButton*> buttons;
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

void TabControl::get_content_min_size(int& w, int& h) const {
	w = 0;
	h = 0;

	for (const auto& p: pages)
		if (p.child) {
			int _w, _h;
			p.child->get_content_min_size(_w, _h);
			w = max(w, _w);
			h = max(h, _h);
		}

	{
		int _w, _h;
		header->get_content_min_size(_w, _h);
		h += _h + Theme::_default.spacing;
	}
}

void TabControl::negotiate_area(const rect& available) {
	_area = available;
	{
		int w, h;
		header->get_content_min_size(w, h);
		header->negotiate_area({available.p00(), available.p10() + vec2(0, h)});
	}
	for (auto& p: pages)
		if (p.child)
			p.child->negotiate_area({header->_area.p01() + vec2(0, Theme::_default.spacing), available.p11()});
}

void TabControl::get_greed_factor(float& x, float& y) const {
	x = 0;
	y = 0;
	for (const auto& p: pages)
		if (p.child) {
			float _x, _y;
			p.child->get_greed_factor(_x, _y);
			x = max(x, _x);
			y = max(y, _y);
		}
}

Array<Control*> TabControl::get_children(ChildFilter f) const {
	Array<Control*> children;
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
	}
}

int TabControl::get_int() {
	return current_page;
}

void TabControl::set_int(int i) {
	current_page = i;
	request_redraw();
}

void TabControl::_draw(Painter* p) {
	header->_draw(p);

	if (pages[current_page].child)
		pages[current_page].child->_draw(p);
}






} // xhui