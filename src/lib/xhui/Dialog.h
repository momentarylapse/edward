//
// Created by Michael Ankele on 2025-01-26.
//

#ifndef DIALOG_H
#define DIALOG_H

#include <lib/base/future.h>

#include "Panel.h"

namespace xhui {

class DialogHeader;
class DialogOutside;

enum class DialogFlags {
	None = 0,
	NoHeader = 1,
	CloseByEscape = 2,
	CloseByClickOutside = 4
};
DialogFlags operator|(DialogFlags a, DialogFlags b);
bool operator&(DialogFlags a, DialogFlags b);

class Dialog : public Panel {
public:
	Dialog(const string& title, int width, int height, Panel* parent, DialogFlags flags = DialogFlags::None);
	Dialog(const string& id, Panel* parent);
	~Dialog() override;
	void negotiate_area(const rect& available) override;
	Array<Control*> get_children(ChildFilter f) const override;
	void _draw(Painter* p) override;

	void request_destroy();
	virtual	void on_destroy() {};

	void set_title(const string& title);

	int width, height;
	DialogFlags flags;
	bool _destroy_requested = false;
	owned<DialogHeader> header;
	owned<DialogOutside> outside;
	base::promise<void> promise;
};

} // xhui

#endif //DIALOG_H
