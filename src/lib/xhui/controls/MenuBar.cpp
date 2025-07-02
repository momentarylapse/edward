//
// Created by Michael Ankele on 2025-04-14.
//

#include "MenuBar.h"
#include "Button.h"
#include <lib/xhui/Theme.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/xhui/dialogs/MenuPopup.h>

namespace xhui {

class MenuBarButton : public Button {
public:
	explicit MenuBarButton(const string& id, const string& title, shared<Menu> _menu) : Button(id, title) {
		size_mode_x = SizeMode::Shrink;
		padding.x1 = padding.x2 = Theme::_default.button_margin_y;
		flat = true;
		menu = _menu;
	}
	void on_click() override {
		/*menu->open_popup_x(owner).then([this] (const string& id) {
			owner->handle_event(id, event_id::Activate, true);
		});*/

		owner->open_dialog(new MenuPopup(*menu.get(), owner, _area, [this] (const string& id) {
			owner->handle_event(id, event_id::Activate, true);
		}));
	}
	shared<Menu> menu;
};

MenuBar::MenuBar(const string& id) : Grid(id) {
	spacing = 3;
}

void MenuBar::_draw(Painter* p) {
	if (is_main) {
		p->set_color(Theme::_default.background_raised());
		p->draw_rect(_area);
	}
	Grid::_draw(p);
}


void MenuBar::set_menu(shared<Menu> _menu) {
	// clear
	while (children.num > 0)
		Grid::remove_child(children.back().control.get());

	menu = _menu;
	for (const auto [i, it]: enumerate(menu->items)) {
		Grid::add_child(new MenuBarButton(it.id, it.title, it.menu), i, 0);
	}
}

void MenuBar::set_option(const string& key, const string& value) {
	if (key == "main") {
		is_main = true;
	} else {
		Grid::set_option(key, value);
	}
}



} // xhui