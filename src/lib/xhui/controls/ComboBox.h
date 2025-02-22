//
// Created by Michael Ankele on 2025-02-12.
//

#ifndef COMBOBOX_H
#define COMBOBOX_H

#include "Button.h"

namespace xhui {

class ComboBox : public Button {
public:
	ComboBox(const string& id, const string& title);

	void reset() override;
	void add_string(const string& s) override;
	void set_int(int i) override;
	int get_int() override;

	vec2 get_content_min_size() const override;
	void on_click() override;
	void _draw(Painter* p) override;

	Array<string> entries;
	int current = 0;
};

} // xhui

#endif //COMBOBOX_H
