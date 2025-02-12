//
// Created by Michael Ankele on 2025-02-11.
//

#ifndef MENU_H
#define MENU_H

#include "Panel.h"
#include "../base/future.h"

namespace xhui {

class Menu : public Sharable<VirtualBase> {
public:
	Menu();

	struct Item {
		string id, title;
		bool enabled = true;
	};
	Array<Item> items;

	void add_item(const string& id, const string& title);

	void open_popup(Panel* p);
	base::future<string> open_popup_x(Panel* p);
};

} // xhui

#endif //MENU_H
