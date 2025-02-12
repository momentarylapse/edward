//
// Created by Michael Ankele on 2025-02-12.
//

#ifndef MENUPOPUP_H
#define MENUPOPUP_H

#include "../Dialog.h"
#include <functional>

namespace xhui {

class Menu;

class MenuPopup : public Dialog {
public:
	explicit MenuPopup(const Menu& m, Panel* parent, const std::function<void(const string&)>& f);

	std::function<void(const string&)> callback;
};

} // xhui

#endif //MENUPOPUP_H
