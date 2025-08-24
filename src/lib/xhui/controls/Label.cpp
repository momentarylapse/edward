#include "Label.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"

namespace xhui {

Label::Label(const string &_id, const string &t) : Control(_id) {
	text_w = text_h = 0;
	font_size = Theme::_default.font_size;
	bold = false;
	italic = false;
	align = Align::Left;
	margin.x1 = margin.x2 = 0;
	margin.y1 = margin.y2 = Theme::_default.label_margin_y;
	ignore_hover = true;

	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;

	Label::set_string(t);
}

void Label::set_string(const string &s) {
	title = s;
	text_w = text_h = -1;
	request_redraw();
}

vec2 Label::get_content_min_size() const {
	if (image) {
		return {20,20};
	} else {
		if (text_w < 0) {
			if (auto win = get_window())
				ui_scale = win->ui_scale;
			auto face = pick_font(Theme::_default.font_name, bold, italic);
			auto dim = get_cached_text_dimensions(title, face, font_size, ui_scale);
			text_w = dim.bounding_width / ui_scale;
			text_h = dim.inner_height() / ui_scale;
		}
		return vec2(text_w, text_h) + margin.p00() + margin.p11();
	}
}

void Label::_draw(Painter *p) {
	ui_scale = p->ui_scale;

	if (image) {
		prepare_image(image);
		vec2 s1 = _area.size();
		vec2 s2 = image->size();
		float scale = min(s1.x / s2.x, s1.y / s2.y);
		vec2 size = s2 * scale;
		p->set_color(White);
		if (!enabled)
			p->set_color(White.with_alpha(0.35f));
		p->draw_ximage({_area.center() - size/2, _area.center() + size/2}, image);
	} else {
		p->set_color(Theme::_default.text_label);
		if (!enabled)
			p->set_color(Theme::_default.text_disabled);

		p->set_font(Theme::_default.font_name, font_size, bold, italic);
		auto dim = get_cached_text_dimensions(title, p->face, font_size, ui_scale);
		float x = _area.x1 + margin.x1;
		if (align == Align::Center)
			x = _area.center().x - dim.bounding_width / ui_scale / 2;
		else if (align == Align::Right)
			x = _area.x2 - dim.bounding_width / ui_scale - margin.x2;
		p->draw_str({x, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
	}
}

void Label::set_option(const string& key, const string& value) {
	if (key == "image") {
		image = load_image(value);
	} else if (key == "align") {
		if (value == "left")
			align = Align::Left;
		if (value == "center")
			align = Align::Center;
		if (value == "right")
			align = Align::Right;
	} else if (key == "right") {
		align = Align::Right;
	} else if (key == "center") {
		align = Align::Center;
	} else if (key == "bold") {
		bold = true;
	} else if (key == "italic") {
		italic = true;
	} else if (key == "big") {
		font_size = Theme::_default.font_size * 1.7f;
	} else if (key == "small") {
		font_size = Theme::_default.font_size * 0.7f;
	} else {
		Control::set_option(key, value);
	}
	request_redraw();
}


}
