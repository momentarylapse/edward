#include "Label.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"

namespace xhui {

Label::Label(const string &_id, const string &t) : Control(_id) {
	text_w = text_h = 0;
	state = State::DEFAULT;

	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;

	Label::set_string(t);
}

void Label::set_string(const string &s) {
	title = s;
	text_w = text_h = -1;
	request_redraw();
}

void Label::get_content_min_size(int &w, int &h) {
	if (text_w < 0) {
		font::set_font(Theme::_default.font_name, Theme::_default.font_size);
		auto dim = font::get_text_dimensions(title);
		text_w = int(dim.bounding_width);
		text_h = int(dim.inner_height());
	}
	w = text_w + Theme::_default.label_margin_x * 2;
	h = text_h + Theme::_default.label_margin_y * 2;
}

void Label::_draw(Painter *p) {
	p->set_color(Theme::_default.text_label);
	p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
	auto dim = font::get_text_dimensions(title);
	p->draw_str({_area.center().x - dim.bounding_width / ui_scale / 2, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
}

}
