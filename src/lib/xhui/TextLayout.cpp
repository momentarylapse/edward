#include "TextLayout.h"
#include "Theme.h"
#include "xhui.h"

namespace xhui {



TextLayout TextLayout::from_format_string(::Painter* p, const string& s, float font_size) {
	if (font_size < 0)
		font_size = xhui::Theme::_default.font_size;
	vec2 pos = vec2(0, 0);
	TextLayout l;
	auto add = [&] (const string& text, const TextFormat& fmt) {
		auto face = xhui::pick_font(xhui::Theme::_default.font_name, fmt.bold, false);
		face->set_size(fmt.font_size * p->ui_scale);
		const auto dims = face->get_text_dimensions(text);

		//float dy = (fmt.font_size - dims.ascender) / p->ui_scale;
		float dy = fmt.font_size - dims.ascender / p->ui_scale;
		l.parts.add({text, fmt, pos + vec2(0, dy)});

		vec2 size = {dims.bounding_width / p->ui_scale, dims.inner_height() / p->ui_scale};
		l.parts.back().box = {pos.x, pos.x + size.x, pos.y + dy, pos.y + size.y + dy};
		pos += vec2(size.x, 0);
	};

	auto find_next_tag = [&s] (int offset, string& tag) -> int {
		int start = s.find("<", offset);
		if (start < offset)
			return -1;
		int end = s.find(">", start);
		if (end < offset)
			return -1;
		tag = s.sub_ref(start+1, end);
		return start;
	};

	// TODO better format parsing...
	int offset = 0;
	TextFormat cur_fmt = {font_size, base::None, 1.0f, false, false};
	Array<TextFormat> format_stack;
	format_stack.add(cur_fmt);
	string cur_tag;
	Array<string> tag_stack;
	tag_stack.add(cur_tag);
	while (true) {
		string next_tag;
		int next = find_next_tag(offset, next_tag);
		if (next >= offset) {
			// anything before tag?
			if (next > offset)
				add(s.sub_ref(offset, next), cur_fmt);

			if (tag_stack.num > 1 and next_tag == "/" + cur_tag) {
				// pop
				tag_stack.pop();
				format_stack.pop();
				cur_fmt = format_stack.back();
				cur_tag = tag_stack.back();
				offset = next + 2 + next_tag.num;
			} else {
				// push
				cur_tag = next_tag;
				if (cur_tag == "b") {
					cur_fmt.bold = true;
				} else if (cur_tag == "big") {
					cur_fmt.font_size = font_size * 1.5f;
				} else if (cur_tag == "small") {
					cur_fmt.font_size = font_size * 0.7f;
				} else if (cur_tag == "soft") {
					cur_fmt.alpha = 0.33f;
				} else if (cur_tag == "red") {
					cur_fmt.col = color(1, 0.7f, 0.1f, 0.1f);
				} else if (cur_tag == "green") {
					cur_fmt.col = color(1, 0.1f, 0.7f, 0.1f);
				} else if (cur_tag == "blue") {
					cur_fmt.col = color(1, 0.1f, 0.1f, 0.8f);
				} else if (cur_tag == "yellow") {
					cur_fmt.col = Yellow;
				}
				format_stack.add(cur_fmt);
				tag_stack.add(cur_tag);
				offset = next + 2 + cur_tag.num;
			}
		} else {
			add(s.sub_ref(offset), cur_fmt);
			break;
		}
	}
	return l;
}

rect TextLayout::box() const {
	rect r = rect::EMPTY;
	for (const auto& t: parts)
		r = r or t.box;
	return r;
}


void draw_text_layout(::Painter* p, const vec2& pos, const TextLayout& l, const color& fg) {
	for (const auto& t: l.parts) {
		if (t.format.col)
			p->set_color(*t.format.col);
		else
			p->set_color(fg.with_alpha(t.format.alpha));
		p->set_font("", t.format.font_size, t.format.bold, t.format.italic);
		p->draw_str(pos + t.pos, t.text);
	}
}

void draw_text_layout_with_box(::Painter* p, const vec2& pos, const TextLayout& l, const color& fg, const color& bg, float padding, float roundness) {
	p->set_color(bg);
	p->set_roundness(roundness);
	auto box = l.box();
	p->draw_rect(rect(pos + box.p00(), pos + box.p11()).grow(padding));
	p->set_roundness(0);
	draw_text_layout(p, pos, l, fg);
}

}
