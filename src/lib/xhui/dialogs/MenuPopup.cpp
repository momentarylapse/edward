//
// Created by Michael Ankele on 2025-02-12.
//

#include "MenuPopup.h"
#include "../controls/Button.h"
#include "../controls/Grid.h"
#include "../Menu.h"
#include "../xhui.h"
#include "../Theme.h"
#include "../../base/iter.h"

namespace xhui {


MenuPopup::MenuPopup(const Menu& m, Panel* parent, const std::function<void(const string&)>& f) :
		Dialog("", 100, 20, parent, DialogFlags::NoHeader | DialogFlags::CloseByEscape | DialogFlags::CloseByClickOutside) {
	padding = Theme::_default.spacing;
	auto g = new Grid("grid");
	Dialog::add_child(g, 0, 0);
	for (const auto& [i, item]: enumerate(m.items)) {
		g->add_child(new CallbackButton(item.id, item.title, [id=item.id, parent, f, this] {
			if (f)
				f(id);
			parent->handle_event(id, event_id::Click, true);
			request_destroy();
		}), 0, i);
		set_options(item.id, "flat");
	}

	//size_mode_x = SizeMode::ForwardChild;
	//size_mode_y = SizeMode::ForwardChild;
	const vec2 size = Dialog::get_content_min_size();
	width = (int)size.x;
	height = (int)size.y;
}

} // xhui