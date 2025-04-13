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

	shared<Menu> menu;
};

} // xhui

#endif //MENUBAR_H
