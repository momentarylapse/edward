//
// Created by Michael Ankele on 2025-02-12.
//

#ifndef MENUPOPUP_H
#define MENUPOPUP_H

#include "../Dialog.h"
#include <functional>

namespace xhui {
class Grid;
class Menu;

class MenuPopup : public Dialog {
public:
	explicit MenuPopup(const Menu& m, Panel* parent, const rect& anchor, const std::function<void(const string&)>& f);

	std::function<void(const string&)> callback;
	Panel* parent;

	Grid* grid;
	const Menu* menu;
	const Menu* current_sub_menu;

	void set_sub_menu(const Menu* m);
};

} // xhui

#endif //MENUPOPUP_H
