//
// Created by Michael Ankele on 2025-01-26.
//

#ifndef DIALOG_H
#define DIALOG_H

#include "Panel.h"

namespace xhui {

class Dialog : public Panel {
public:
	Dialog(const string& title, int width, int height, Panel* parent);
	~Dialog() override;
	void negotiate_area(const rect& available) override;
	void _draw(Painter* p) override;

	void request_destroy();
	virtual	void on_destroy() {};

	string title;
	int width, height;
	float padding;
	bool _destroy_requested = false;
};

} // xhui

#endif //DIALOG_H
