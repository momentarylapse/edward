//
// Created by Michael Ankele on 2025-02-11.
//

#include "Menu.h"
#include "Dialog.h"
#include "Theme.h"
#include "xhui.h"
#include "controls/Button.h"
#include "controls/Grid.h"
#include "../base/iter.h"

namespace xhui {

class MenuPopup : public Dialog {
public:
	explicit MenuPopup(const Menu& m, Panel* parent) : Dialog("", 100, 200, parent) {
		auto g = new Grid("grid");
		Dialog::add_child(g, 0, 0);
		for (const auto& [i, item]: enumerate(m.items))
			g->add_child(new CallbackButton(item.id, item.title, [id=item.id, parent, this] {
				parent->handle_event(id, event_id::Click, true);
				request_destroy();
			}), 0, i);

		Dialog::get_content_min_size(width, height);
		width += Theme::_default.spacing * 2;
		height += Theme::_default.spacing * 2;
	}
};

Menu::Menu() {
}

void Menu::add_item(const string& id, const string& title) {
	items.add({id, title});
}

void Menu::open_popup(Panel* p) {
	if (!p->get_window())
		return;
	p->open_dialog(new MenuPopup(*this, p));
}



} // xhui