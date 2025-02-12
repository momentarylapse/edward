//
// Created by Michael Ankele on 2025-02-11.
//

#include "Menu.h"
#include "dialogs/MenuPopup.h"

namespace xhui {

Menu::Menu() {
}

void Menu::add_item(const string& id, const string& title) {
	items.add({id, title});
}

void Menu::open_popup(Panel* p) {
	if (!p->get_window())
		return;
	p->open_dialog(new MenuPopup(*this, p, nullptr));
}


} // xhui