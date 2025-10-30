//
// Created by Michael Ankele on 2025-02-12.
//

#include "MenuPopup.h"

#include <lib/base/set.h>

#include "../controls/Button.h"
#include "../controls/Grid.h"
#include "../Menu.h"
#include "../xhui.h"
#include "../Theme.h"
#include "../../base/iter.h"

namespace xhui {


MenuPopup::MenuPopup(const Menu& m, Panel* _parent, const rect& anchor, const std::function<void(const string&)>& f) :
		Dialog("", 100, 20, _parent, DialogFlags::NoHeader | DialogFlags::CloseByEscape | DialogFlags::CloseByClickOutside | DialogFlags::FixedPosition) {
	id = "menu-popup";
	padding = Theme::_default.spacing;
	pos = anchor.p01();
	grid = new Grid("grid");
	Dialog::add_child(grid, 0, 0);
	menu = &m;
	parent = _parent;
	callback = f;

	owner = parent; // can only call get_content_min_size() with an owner...
	set_sub_menu(menu);
	if (auto w = get_window()) {
		pos.y = max(min(pos.y, w->_area.y2 - (float)height), w->_area.y1);
		pos.x = max(min(pos.x, w->_area.x2 - (float)width), w->_area.x1);
	}
	owner = nullptr;
}

void MenuPopup::set_sub_menu(const Menu* m) {
	current_sub_menu = m;

	auto xx= grid->get_children(ChildFilter::All);
	for (auto c: xx)
		grid->remove_child(c);

	for (const auto& [i, item]: enumerate(m->items)) {
		if (item.menu) {
			grid->add_child(new CallbackButton(item.id, item.title + " >>", [mm=item.menu.get(), this] {
				set_sub_menu(mm);
			}), 0, i);
		} else {
			grid->add_child(new CallbackButton(item.id, item.title, [id=item.id, this] {
				if (callback)
					callback(id);
				request_destroy();
			}), 0, i);
		}
		set_options(item.id, "flat,align=left");
		enable(item.id, item.enabled);
	}

	//size_mode_x = SizeMode::ForwardChild;
	//size_mode_y = SizeMode::ForwardChild;
	const vec2 size = Dialog::get_content_min_size();
	width = (int)size.x;
	height = (int)size.y;
}


} // xhui