//
// Created by michi on 10/30/25.
//

#pragma once

#include "../Dialog.h"

namespace xhui {

class AboutDialog : public Dialog {
public:
	AboutDialog(Panel* parent);

	static void show(Panel* parent);
};

}
