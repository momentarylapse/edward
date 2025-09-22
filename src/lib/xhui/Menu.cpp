//
// Created by Michael Ankele on 2025-02-11.
//

#include "Menu.h"

#include "Window.h"
#include "dialogs/MenuPopup.h"

namespace xhui {

Menu::Menu() {
}

string strip_options(const string& t) {
	if (t.head(1) == "!")
		return t.explode("\\")[1];
	return t;
}

void Menu::add_item(const string& id, const string& title) {
	items.add({id, strip_options(title)});
}

void Menu::add_item_menu(const string& id, const string& title, shared<Menu> menu) {
	items.add({id, strip_options(title), menu});
}

void Menu::enable(const string& id, bool enabled) {
	for (auto& item: items)
		if (item.id == id)
			item.enabled = enabled;
}


void Menu::open_popup(Panel* p) {
	if (!p->get_window())
		return;
	const vec2 m = p->get_window()->mouse_position();
	p->open_dialog(new MenuPopup(*this, p, {m, m}, nullptr));
}

} // xhui