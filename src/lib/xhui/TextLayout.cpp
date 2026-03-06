#include "TextLayout.h"
#include "Theme.h"
#include "xhui.h"
#include "lib/os/msg.h"

namespace xhui {

struct TagInfo {
	int offset, end;
	string tag;
	base::map<string, string> attributes;
};

base::optional<TagInfo> find_next_tag(const string& s, int offset) {
	int start = s.find("<", offset);
	if (start < offset)
		return base::None;
	int end = s.find(">", start);
	if (end < offset)
		return base::None;
	TagInfo t;
	t.tag = s.sub_ref(start+1, end);
	t.offset = start;
	t.end = end + 1;

	if (t.tag.find(" ") >= 0) {
		auto tokens = t.tag.parse_tokens("=");
		t.tag = tokens[0];
		for (int i=1; i<tokens.num-2; i+=3) {
			if (tokens[i+1] != "=")
				break;
			t.attributes.set(tokens[i], tokens[i+2]);
		}
	}

	return t;
};

color parse_color(const string& v) {
	if (v == "white")
		return White;
	if (v == "black")
		return Black;
	if (v == "red")
		return color(1, 0.7f, 0.1f, 0.1f);;
	if (v == "green")
		return color(1, 0.1f, 0.7f, 0.1f);
	if (v == "blue")
		return color(1, 0.1f, 0.1f, 0.8f);
	if (v == "yellow")
		return Yellow;
	return color::parse(v);
}

TextFormat apply_tag(const TextFormat& parent, const TagInfo& tag, float font_size) {
	auto f = parent;
	if (tag.tag == "b") {
		f.bold = true;
	} else if (tag.tag == "i") {
		f.italic = true;
	} else if (tag.tag == "small") {
		f.font_size = font_size * 0.7f;
	} else if (tag.tag == "big") {
		f.font_size = font_size * 1.44f;
	} else if (tag.tag == "huge") {
		f.font_size = font_size * 2.1f;
	} else if (tag.tag == "span" or tag.tag == "font") {
		for (const auto& [k, v]: tag.attributes) {
			if (k == "color" or k == "foreground" or k == "fgcolor") {
				f.col = parse_color(v);
			} if (k == "alpha") {
				if (v.tail(1) == "%")
					f.alpha = (float)v.replace("%", "")._int() / 100.0f;
			} if (k == "size") {
				if (v.tail(1) == "%")
					f.font_size = font_size * (float)v.replace("%", "")._int() / 100.0f;
				else if (v == "small")
					f.font_size = font_size * 0.7f;
				else if (v == "big")
					f.font_size = font_size * 1.44f;
				else if (v == "huge")
					f.font_size = font_size * 2.1f;
			} if (k == "weight") {
				if (v == "normal")
					f.bold = false;
				else if (v == "bold")
					f.bold = true;
			}
		}
	}
	return f;
}

TextLayout TextLayout::from_format_string(::Painter* p, const string& s, float font_size, int align) {
	if (font_size < 0)
		font_size = xhui::Theme::_default.font_size;
	vec2 pos = vec2(0, 0);
	TextLayout l;
	float max_line_width = 0;
	float max_line_height = 0;
	float max_line_ascender = 0;
	int line_first_chunk = 0;
	auto add_chunk = [&] (const string& text, const TextFormat& fmt) {
		auto face = xhui::pick_font(xhui::Theme::_default.font_name, fmt.bold, false);
		face->set_size(fmt.font_size * p->ui_scale);
		const auto dims = face->get_text_dimensions(text);

		max_line_height = max(max_line_height, dims.line_dy / p->ui_scale);
		max_line_ascender = max(max_line_ascender, dims.ascender / p->ui_scale);

		float dy =  - dims.ascender / p->ui_scale;
		l.parts.add({text, fmt, pos + vec2(0, dy)});

		vec2 size = {dims.bounding_width / p->ui_scale, dims.inner_height() / p->ui_scale};
		l.parts.back().box = {pos.x, pos.x + size.x, pos.y + dy, pos.y + size.y + dy};
		pos += vec2(size.x, 0);
	};

	int offset = 0;
	TextFormat cur_fmt = {font_size, base::None, 1.0f, false, false};
	Array<TextFormat> format_stack;
	format_stack.add(cur_fmt);
	string cur_tag;
	Array<string> tag_stack;
	tag_stack.add(cur_tag);

	auto end_line = [&] {
		max_line_width = max(max_line_width, pos.x);
		// correct previous line
		vec2 d = {0, max_line_ascender};
		if (align == 0)
			d.x = - pos.x / 2;
		else if (align == 1)
			d.x = - pos.x;
		for (int i=line_first_chunk; i<l.parts.num; i++) {
			l.parts[i].pos += d;
			l.parts[i].box += d;
		}
		// start next line
		pos.x = 0;
		pos.y += max_line_height;
		max_line_height = 0;
		max_line_ascender = 0;
		line_first_chunk = l.parts.num;
	};

	auto handle_text_no_tags = [&] (int end) {
		while (true) {
			int p0 = s.find("\n", offset);
			if (p0 >= offset and p0 < end) {
				if (p0 > offset)
					add_chunk(s.sub_ref(offset, p0), cur_fmt);
				offset = p0 + 1;
				end_line();
			} else {
				if (offset < end)
					add_chunk(s.sub_ref(offset, end), cur_fmt);
				break;
			}
		}
	};


	while (true) {
		if (auto next_tag = find_next_tag(s, offset)) {
			// anything before tag?
			if (next_tag->offset > offset)
				handle_text_no_tags(next_tag->offset);

			if (tag_stack.num > 1 and next_tag->tag == "/" + cur_tag) {
				// pop
				tag_stack.pop();
				format_stack.pop();
				cur_fmt = format_stack.back();
				cur_tag = tag_stack.back();
			} else {
				// push
				cur_tag = next_tag->tag;
				cur_fmt = apply_tag(cur_fmt, *next_tag, font_size);
				format_stack.add(cur_fmt);
				tag_stack.add(cur_tag);
			}
			offset = next_tag->end;
		} else {
			handle_text_no_tags(s.num);
			end_line();
			break;
		}
	}

	if (align != -1) {
		vec2 d = {(align == 0) ? max_line_width/2 : max_line_width, 0};
		for (auto& c: l.parts) {
			c.pos += d;
			c.box += d;
		}
	}

	l.box = l.compute_box();
	return l;
}

rect TextLayout::compute_box() const {
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
	p->draw_rect(rect(pos + l.box.p00(), pos + l.box.p11()).grow(padding));
	p->set_roundness(0);
	draw_text_layout(p, pos, l, fg);
}

}
