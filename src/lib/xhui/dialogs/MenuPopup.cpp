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


MenuPopup::MenuPopup(const Menu& m, Panel* parent, const std::function<void(const string&)>& f) : Dialog("", 100, 200, parent) {
	auto g = new Grid("grid");
	Dialog::add_child(g, 0, 0);
	for (const auto& [i, item]: enumerate(m.items))
		g->add_child(new CallbackButton(item.id, item.title, [id=item.id, parent, f, this] {
			if (f)
				f(id);
			parent->handle_event(id, event_id::Click, true);
			request_destroy();
		}), 0, i);

	vec2 size = Dialog::get_content_min_size();
	width = (int)(size.x + Theme::_default.spacing * 2);
	height = (int)(size.y + Theme::_default.spacing * 2);
}

} // xhui