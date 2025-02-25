#include "Edit.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"
#include "../../base/iter.h"
#include "../../os/msg.h"

namespace xhui {

Edit::Edit(const string &_id, const string &t) : Control(_id) {
	//state = State::DEFAULT;
	can_grab_focus = true;

	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;

	Edit::set_string(t);
}

void Edit::Cache::rebuild(const string& text) {
	*this = {};
	lines = text.explode("\n");
	if (lines.num == 0)
		lines.add("");
	int index = 0;
	for (const string& l: lines) {
		line_num_characters.add(l.num);
		line_first_index.add(index);
		index += l.num + 1;
	}
}

void Edit::set_string(const string &s) {
	text = s;
	cache.rebuild(text);
	cursor_pos = text.num;
	request_redraw();
}

void Edit::enable(bool _enabled) {
	enabled = _enabled;
	request_redraw();
}

string Edit::get_string() {
	return text;
}


vec2 Edit::get_content_min_size() const {
	return {80, 30};
}

void Edit::on_key_down(int key) {
	if (!enabled) {
		request_redraw();
		return;
	}
	const auto cur_lp = index_to_line_pos(cursor_pos);

	if (key == KEY_LEFT)
		cursor_pos = clamp(cursor_pos - 1, 0, text.num);
	if (key == KEY_RIGHT)
		cursor_pos = clamp(cursor_pos + 1, 0, text.num);
	if (key == KEY_HOME)
		cursor_pos = cache.line_first_index[cur_lp.line];
	if (key == KEY_END)
		cursor_pos = cache.line_first_index[cur_lp.line] + cache.line_num_characters[cur_lp.line];

	auto jump_lines = [this, cur_lp] (int dlines) {
		cursor_pos = line_pos_to_index({cur_lp.line + dlines, cur_lp.offset});
	};
	if (key == KEY_UP and multiline)
		jump_lines(-1);
	if (key == KEY_DOWN and multiline)
		jump_lines(1);

	if (key == KEY_BACKSPACE)
		if (cursor_pos > 0) {
			text = text.sub_ref(0, cursor_pos - 1) + text.sub_ref(cursor_pos);
			cache.rebuild(text);
			cursor_pos --;
			on_edit();
			emit_event(event_id::Changed, true);
		}
	if (key == KEY_DELETE)
		if (cursor_pos < text.num) {
			text = text.sub_ref(0, cursor_pos) + text.sub_ref(cursor_pos + 1);
			cache.rebuild(text);
			on_edit();
			emit_event(event_id::Changed, true);
		}

	auto insert = [this] (char c) {
		text = text.sub_ref(0, cursor_pos) + string(&c, 1) + text.sub_ref(cursor_pos);
		cache.rebuild(text);
		cursor_pos ++;
		on_edit();
		emit_event(event_id::Changed, true);
	};
	if (key >= KEY_0 and key <= KEY_9)
		insert('0' + (key - KEY_0));
	if (key == KEY_DOT)
		if (text.find(".") < 0 or !numerical)
			insert('.');
	if (key == KEY_MINUS)
		insert('-');
	if (!numerical) {
		if (key >= KEY_A and key <= KEY_Z)
			insert('a' + (key - KEY_A));
		if (key == KEY_SPACE)
			insert(' ');
		if (key == KEY_COMMA)
			insert(',');
		if (key == KEY_PLUS)
			insert('+');
	}
	if (key == KEY_RETURN and multiline)
		insert('\n');

	request_redraw();
}

void Edit::draw_active_marker(Painter* p) {

}

void Edit::draw_text(Painter* p) {
	p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);

	// update text dims
	float inner_height = 0;
	{
		auto& lines = cache.lines;
		cache.line_y0.clear();
		cache.line_height.clear();
		cache.line_width.clear();
		float y0 = _area.y1 + 8;
		for (const string &l: lines) {
			auto dim = default_font_regular->get_text_dimensions(l);
			inner_height = dim.inner_height() / ui_scale;
			cache.line_height.add(dim.line_dy / ui_scale);
			cache.line_y0.add(y0);
			cache.line_width.add(dim.dx / ui_scale);
			y0 += dim.line_dy / ui_scale;
		}
		if (!multiline)
			cache.line_y0[0] = _area.center().y - inner_height / 2;
	}


	// text
	float x0 = _area.x1 + Theme::_default.edit_margin_x;
	p->set_color(Theme::_default.text_label);
	if (!enabled)
		p->set_color(Theme::_default.text_disabled);
	for (const auto& [line, l]: enumerate(cache.lines)) {
		p->draw_str({x0, cache.line_y0[line]}, l);
	}

	// cursor
	if (has_focus() and enabled) {
		p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
		auto lp = index_to_line_pos(cursor_pos);
		int first = cache.line_first_index[lp.line];
		auto dim = default_font_regular->get_text_dimensions(text.sub_ref(first, cursor_pos));
		//p->set_color(Theme::_default.text_label);
		float x = x0 + dim.bounding_width / ui_scale;
		float y0 = cache.line_y0[lp.line];
		p->draw_line({x, y0 - 3}, {x, y0 + Theme::_default.font_size + 3});
	}
}


void Edit::_draw(Painter *p) {

	// background
	color bg = Theme::_default.background_button;
	p->set_color(bg);
	p->set_roundness(Theme::_default.button_radius);
	p->draw_rect(_area);

	// focus frame
	if (has_focus()) {
		p->set_color(Theme::_default.background_button_primary.with_alpha(0.6f));
		p->draw_rect(_area);

		float dr = Theme::_default.focus_frame_width;
		p->set_roundness(Theme::_default.button_radius - dr);
		p->set_color(bg);
		p->draw_rect(smaller_rect(_area, dr));
	}
	p->set_line_width(1);
	p->set_roundness(0);

	draw_text(p);
}
Edit::LinePos Edit::index_to_line_pos(int index) const {
	LinePos r = {0, 0};
	for (const auto& [line, first]: enumerate(cache.line_first_index)) {
		if (index < first)
			break;
		r = {line, index - first};
	}
	return r;
}

int Edit::line_pos_to_index(const LinePos& lp) const {
	if (lp.line < 0)
		return 0;
	if (lp.line >= cache.lines.num)
		return text.num;
	return cache.line_first_index[lp.line] + clamp(lp.offset, 0, cache.line_num_characters[lp.line]);
}

}
