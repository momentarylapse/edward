//
// Created by michi on 03.02.25.
//

#ifndef COLORSELECTIONDIALOG_H
#define COLORSELECTIONDIALOG_H

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
	ColorIndicator* indicator;
	base::promise<color> promise;

	static base::future<color> ask(Panel* parent, const string& title, const color& col, const Array<string>& params);
};

} // xhui

#endif //COLORSELECTIONDIALOG_H
