#include "Label.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../TextLayout.h"
#include <lib/ygraphics/font.h>

namespace xhui {

Label::Label(const string &_id, const string &t) : Control(_id) {
	text_w = text_h = -1;
	font_size = Theme::_default.font_size;
	align = Align::Left;
	padding.x1 = padding.x2 = 0;
	padding.y1 = padding.y2 = Theme::_default.label_margin_y;
	ignore_hover = true;

	size_mode_x = SizeMode::Fill;
	size_mode_y = SizeMode::Fill;

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
			if (markup and false) {
				//auto l = TextLayout::from_format_string(title, );

			} else {
				auto face = pick_font(Theme::_default.font_name, bold, italic);
				auto dim = get_cached_text_dimensions(title, face, font_size, ui_scale);
				text_w = dim.bounding_width / ui_scale;
				text_h = dim.inner_height() / ui_scale;
			}
		}
		if (ellipsis)
			return {20, text_h};
		return {text_w, text_h};
	}
}

void Label::_draw(Painter *p) {
	ui_scale = p->ui_scale;

	if (image) {
		prepare_image(image);
		vec2 s1 = area.size();
		vec2 s2 = image->size();
		float scale = min(s1.x / s2.x, s1.y / s2.y);
		vec2 size = s2 * scale;
		p->set_color(White);
		if (!enabled)
			p->set_color(White.with_alpha(0.35f));
		p->draw_ximage({area.center() - size/2, area.center() + size/2}, image);
	} else if (markup) {
		auto l = TextLayout::from_format_string(p, title, font_size);
		text_h = l.box.height();
		text_w = l.box.width();
		draw_text_layout(p, area.p00() + padding.p00() - l.box.p00(), l, enabled ? Theme::_default.text_label : Theme::_default.text_disabled);
	} else {
		if (url)
			p->set_color(Theme::_default.text_link);
		else
			p->set_color(Theme::_default.text_label);
		if (!enabled)
			p->set_color(Theme::_default.text_disabled);

		p->set_font(Theme::_default.font_name, font_size, bold, italic);
		auto title_eff = title;
		auto dim = get_cached_text_dimensions(title, p->face, font_size, ui_scale);
		if (ellipsis and dim.bounding_width / ui_scale > area.width() - padding.width()) {
			for (int n=title.num/2-1; n>=2; n--) {
				title_eff = title.head(n) + "..." + title.tail(n);
				dim = get_cached_text_dimensions(title_eff, p->face, font_size, ui_scale);
				if (dim.bounding_width / ui_scale <= area.width() - padding.width())
					break;
			}
		}
		float x = area.x1 + padding.x1;
		if (align == Align::Center)
			x = area.center().x - dim.bounding_width / ui_scale / 2;
		else if (align == Align::Right)
			x = area.x2 - dim.bounding_width / ui_scale - padding.x2;
		p->draw_str({x, area.center().y - dim.inner_height() / ui_scale / 2}, title_eff);
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
		bold = (value == "") ? true : value._bool();
	} else if (key == "italic") {
		italic = true;
	} else if (key == "big") {
		font_size = Theme::_default.font_size_big;
	} else if (key == "small") {
		font_size = Theme::_default.font_size_small;
	} else if (key == "url") {
		url = true;
	} else if (key == "markup") {
		markup = true;
	} else if (key == "ellipsis") {
		ellipsis = true;
	} else {
		Control::set_option(key, value);
	}
	request_redraw();
}


}
