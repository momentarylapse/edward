#include "Label.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"

namespace xhui {

Label::Label(const string &_id, const string &t) : Control(_id) {
	text_w = text_h = 0;
	align = Align::Left;
	margin_x = 0;

	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;

	Label::set_string(t);
}

void Label::set_string(const string &s) {
	title = s;
	text_w = text_h = -1;
	request_redraw();
}

void Label::get_content_min_size(int &w, int &h) const {
	if (text_w < 0) {
		font::set_font(Theme::_default.font_name, Theme::_default.font_size * ui_scale);
		auto dim = font::get_text_dimensions(title);
		text_w = int(dim.bounding_width / ui_scale);
		text_h = int(dim.inner_height() / ui_scale);
	}
	w = text_w + margin_x * 2;
	h = text_h + Theme::_default.label_margin_y * 2;
}

void Label::_draw(Painter *p) {
	if (image) {
		prepare_image(image);
		vec2 size = _area.size() - vec2(16, 16);
		p->set_color(White);
		if (!enabled)
			p->set_color(White.with_alpha(0.35f));
		p->draw_ximage({_area.center() - size/2, _area.center() + size/2}, image);
	} else {
		p->set_color(Theme::_default.text_label);
		if (!enabled)
			p->set_color(Theme::_default.text_disabled);

		p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
		auto dim = font::get_text_dimensions(title);
		float x = _area.x1 + margin_x;
		if (align == Align::Center)
			x = _area.center().x - dim.bounding_width / ui_scale / 2;
		else if (align == Align::Right)
			x = _area.x2 - dim.bounding_width / ui_scale - margin_x;
		p->draw_str({x, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
	}
}

void Label::set_option(const string& key, const string& value) {
	if (key == "image") {
		image = load_image(value);
		request_redraw();
	} else if (key == "align") {
		if (value == "left")
			align = Align::Left;
		if (value == "center")
			align = Align::Center;
		if (value == "right")
			align = Align::Right;
	} else {
		Control::set_option(key, value);
	}
}


}
