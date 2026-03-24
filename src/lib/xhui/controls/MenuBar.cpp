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

void open_popup_menu(Control* c, shared<Menu> menu) {
	c->owner->open_dialog(new MenuPopup(menu, c->owner, c->area, [c] (const string& id) {
		// wait for the popup to close
		run_later(0.01f, [c, id] {
			c->owner->handle_event(id, event_id::Activate, true);
		});
	}));
}

class MenuBarButton : public Button {
public:
	explicit MenuBarButton(const string& id, const string& title, shared<Menu> _menu) : Button(id, title) {
		size_mode_x = SizeMode::Fill;
		padding.x1 = padding.x2 = Theme::_default.button_margin_y;
		flat = true;
		menu = _menu;
	}
	void on_click() override {
		open_popup_menu(this, menu);
	}
	shared<Menu> menu;
};

MenuBar::MenuBar(const string& id) : Grid(id) {
	grid.spacing = 3;
}

void MenuBar::_draw(Painter* p) {
	if (is_main) {
		p->set_color(Theme::_default.background_raised());
		p->draw_rect(area);
	}
	Grid::_draw(p);
}


void MenuBar::set_menu(shared<Menu> _menu) {
	// clear
	while (grid.children.num > 0)
		Grid::remove_child((Control*)grid.children.back().node.get());

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