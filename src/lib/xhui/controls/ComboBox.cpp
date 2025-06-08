//
// Created by Michael Ankele on 2025-02-12.
//

#include "ComboBox.h"
#include "../Theme.h"
#include "../xhui.h"
#include "../Menu.h"
#include "../dialogs/MenuPopup.h"
#include "../../base/iter.h"

namespace xhui {

constexpr float BUTTON_DX = 30;

ComboBox::ComboBox(const string& id, const string& title) : Button(id, "") {
	label.align = Label::Align::Left;
	entries = title.explode("\\");
	current = 0;
	if (entries.num >= 1)
		Button::set_string(entries[0]);
}

void ComboBox::reset() {
	entries.clear();
	current = 0;
}

void ComboBox::add_string(const string& s) {
	entries.add(s);
}

int ComboBox::get_int() {
	return current;
}

void ComboBox::set_int(int i) {
	if (i < 0 or i >= entries.num)
		return;
	current = i;
	Button::set_string(entries[current]);
}

vec2 ComboBox::get_content_min_size() const {
	return Button::get_content_min_size() + vec2(BUTTON_DX, 0);
}


void ComboBox::on_click() {
	Menu menu;
	for (const auto& [i, e]: enumerate(entries))
		menu.add_item(format(":combo:%d", i), e);
	owner->open_dialog(new MenuPopup(menu, owner, _area, [this] (const string& id) {
		int n = id.sub_ref(7)._int();
		set_int(n);
		emit_event(event_id::Changed, true);
	}));
}

void ComboBox::_draw(Painter* p) {
	Button::_draw(p);
	float dx = BUTTON_DX;

	const rect area_arrow = {_area.p10() - vec2(dx,0), _area.p11()};

	p->set_color(Theme::_default.text_label);
	if (!enabled)
		p->set_color(Theme::_default.text_disabled);
	/*float adx = 5.5f;
	float ady = 4;
	p->set_line_width(2);
	p->draw_lines({area_arrow.center() + vec2(-adx, -ady),
		area_arrow.center() + vec2(adx, -ady),
		area_arrow.center() + vec2(0, ady),
		area_arrow.center() + vec2(-adx, -ady)});
	p->set_line_width(1);*/

	auto im = load_image("hui:triangle-down");
	prepare_image(im);
	p->draw_ximage(area_arrow, im);
}


} // xhui