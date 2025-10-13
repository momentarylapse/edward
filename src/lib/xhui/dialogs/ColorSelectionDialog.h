//
// Created by michi on 03.02.25.
//

#pragma once

#include <lib/base/optional.h>

#include "../Dialog.h"
#include "../../base/future.h"
#include "../../image/color.h"

namespace xhui {

class ColorHSBSelector;
class ColorIndicator;

class ColorSelectionDialog : public Dialog {
public:
	ColorSelectionDialog(Panel* parent, const string& title, const color& col, const Array<string>& params);

	ColorHSBSelector* selector;
	ColorIndicator* indicator_original;
	ColorIndicator* indicator;
	base::optional<color> answer;
	base::promise<color> promise;

	static base::future<color> ask(Panel* parent, const string& title, const color& col, const Array<string>& params);
};

} // xhui
