//
// Created by Michael Ankele on 2025-04-14.
//

#ifndef MENUBAR_H
#define MENUBAR_H

#include "Grid.h"
#include "../Menu.h"

namespace xhui {

class MenuBar : public Grid {
public:
	explicit MenuBar(const string &id);

	void set_menu(shared<Menu> menu);
	void _draw(Painter* p) override;
	void set_option(const string& key, const string& value) override;

	shared<Menu> menu;
	bool is_main = false;
};

} // xhui

#endif //MENUBAR_H
