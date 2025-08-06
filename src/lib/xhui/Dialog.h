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
	CloseByClickOutside = 4,
	FixedPosition = 8
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

	void on_key_down(int key) override;

	void request_destroy();
	virtual	void on_destroy() {};

	void set_title(const string& title);
	rect suggest_area(const rect& parent_area) const;

	int width, height;
	vec2 pos;
	DialogFlags flags;
	bool _destroy_requested = false;
	owned<DialogHeader> header;
	owned<DialogOutside> outside;
	base::promise<void> basic_promise;
};

Dialog* as_dialog(Control* c);

} // xhui

#endif //DIALOG_H
