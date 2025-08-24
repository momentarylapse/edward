#include "Edit.h"

#include <lib/base/algo.h>
#include <lib/base/sort.h>
#include <lib/os/time.h>

#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"
#include "../../base/iter.h"
#include "../../os/msg.h"

namespace xhui {


//#define PERF_OUT


FontFlags operator|(FontFlags a, FontFlags b) {
	return (FontFlags)((int)a | (int)b);
}
bool operator&(FontFlags a, FontFlags b) {
	return (int)a & (int)b;
}

int next_utf8_index(const string& text, int index) {
	for (int i=index; i<min(index + 8, text.num); i++)
		if ((text[i] & 0x80) == 0x00 or (text[i] & 0xc0) == 0x80)
			return i + 1;
	return index;
}

int prior_utf8_index(const string& text, int index) {
	for (int i=max(index-1,0); i>=index-8; i--)
		if ((text[i] & 0x80) == 0x00 or (text[i] & 0xc0) == 0xc0)
			return i;
	return index;
}

Edit::Edit(const string &_id, const string &t) : Control(_id) {
	//state = State::DEFAULT;
	can_grab_focus = true;

	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;

	font_name = Theme::_default.font_name;
	font_size = Theme::_default.font_size;

	margin_x = Theme::_default.edit_margin_x;
	margin_y = 8;

	tab_size = 4;
	face = nullptr;

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
	//set_cursor_pos(0); // depends on cache update (after on_draw!)
	cursor_pos = 0;
	selection_start = 0;
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

void Edit::on_left_button_down(const vec2& m) {
	set_cursor_pos(xy_to_index(m), get_window()->is_key_pressed(KEY_SHIFT));
}

void Edit::on_mouse_move(const vec2& m, const vec2& d) {
	if (owner->get_window()->button(0)) {
		set_cursor_pos(xy_to_index(m), true);
	}
}

vec2 Edit::viewport_size() const {
	return vec2::max(cache.content_size - _area.size() + vec2(margin_x * 2, 0), vec2::ZERO);
}


void Edit::on_mouse_wheel(const vec2& d) {
	viewport_offset = vec2::max(vec2::min(viewport_offset - d * 20, viewport_size()), vec2::ZERO);
	request_redraw();
	emit_event(event_id::Scroll, false);
}


void Edit::on_key_down(int key) {
	if (!enabled) {
		request_redraw();
		return;
	}
	const auto cur_lp = index_to_line_pos(cursor_pos);

	bool shift = (key & KEY_SHIFT);
	int key_no_shift = key & ~KEY_SHIFT;

	if (key_no_shift == KEY_LEFT)
		set_cursor_pos(clamp(prior_index(cursor_pos), 0, text.num), shift);
	if (key_no_shift == KEY_RIGHT)
		set_cursor_pos(clamp(next_index(cursor_pos), 0, text.num), shift);
#ifdef OS_MAC
	if (key_no_shift == KEY_LEFT + KEY_ALT)
#else
	if (key_no_shift == KEY_HOME)
#endif
		set_cursor_pos(cache.line_first_index[cur_lp.line], shift);
#ifdef OS_MAC
	if (key_no_shift == KEY_RIGHT + KEY_ALT)
#else
	if (key_no_shift == KEY_END)
#endif
		set_cursor_pos(cache.line_first_index[cur_lp.line] + cache.line_num_characters[cur_lp.line], shift);

	auto jump_lines = [this, cur_lp, shift] (int dlines) {
		set_cursor_pos(line_pos_to_index({cur_lp.line + dlines, cur_lp.offset}), shift);
	};
	if (multiline) {
		if (key_no_shift == KEY_UP)
			jump_lines(-1);
		if (key_no_shift == KEY_DOWN)
			jump_lines(1);
		if (key_no_shift == KEY_PAGE_UP)
			jump_lines(- (int)(_area.height() / cache.line_height[0]));
		if (key_no_shift == KEY_PAGE_DOWN)
			jump_lines((int)(_area.height() / cache.line_height[0]));
	}


#ifdef OS_MAC
	int mod = KEY_SUPER;
#else
	int mod = KEY_CONTROL;
#endif

	if (key == KEY_C + mod)
		clipboard::copy(get_range(selection_start, cursor_pos));
	if (key == KEY_V + mod)
		auto_insert(clipboard::paste());
	if (key == KEY_X + mod) {
		clipboard::copy(get_range(selection_start, cursor_pos));
		delete_selection();
	}
	if (key == KEY_Z + mod and current_history_index > 0) {
		auto& op = history[-- current_history_index];
		string old = get_range(op.i0, op.i1);
		_replace_range(op.i0, op.i1, op.t);
		op.i1 = op.i0 + op.t.num;
		op.t = old;
	}
	if (key == KEY_Y + mod and current_history_index < history.num) {
		auto& op = history[current_history_index ++];
		string old = get_range(op.i0, op.i1);
		_replace_range(op.i0, op.i1, op.t);
		op.i1 = op.i0 + op.t.num;
		op.t = old;
	}

	if (key == KEY_BACKSPACE) {
		if (cursor_pos != selection_start) {
			delete_selection();
		} else if (cursor_pos > 0) {
			delete_range(prior_index(cursor_pos), cursor_pos);
		}
	}
	if (key == KEY_DELETE) {
		if (cursor_pos != selection_start) {
			delete_selection();
		} else if (cursor_pos < text.num) {
			delete_range(cursor_pos, next_index(cursor_pos));
		}
	}

	if (key == KEY_RETURN) {
		if (multiline)
			auto_insert("\n");
		else
			emit_event(event_id::ActivateDialogDefault, false);
	}
	if (key == KEY_TAB and multiline) {
		if (index_to_line_pos(selection_start).line != index_to_line_pos(cursor_pos).line) {
			multi_line_indent(1);
		} else {
			auto_insert("\t");
		}
	}
	if (key == (KEY_TAB | KEY_SHIFT) and multiline) {
		multi_line_indent(-1);
	}

	request_redraw();
}

void Edit::multi_line_indent(int indent) {
	int line0 = index_to_line_pos(min(selection_start, cursor_pos)).line;
	int line1 = index_to_line_pos(max(selection_start, cursor_pos)).line;
	for (int l=line0; l<=line1; l++) {
		int p = line_pos_to_index({l, 0});
		if (indent > 0) {
			replace_range(p, p, "\t");
		} else {
			if (get_range(p, p + 1) == "\t")
				replace_range(p, p + 1, "");
			else if (get_range(p, p + tab_size) == string(" ").repeat(tab_size))
				replace_range(p, p + tab_size, "");
		}
	}
}

void Edit::on_key_char(int character) {
	if (!enabled) {
		request_redraw();
		return;
	}

	if (character != '\n' or multiline)
		auto_insert(utf32_to_utf8({character}));

	request_redraw();
}

void Edit::draw_active_marker(Painter* p) {

}

void Edit::draw_text(Painter* p) {
#ifdef PERF_OUT
	os::Timer timer;
#endif

	const auto clip0 = p->clip();
	p->set_clip(_area);
	p->set_font(font_name, font_size, false, false);
	face = p->face;

	text_x0 = _area.x1 + margin_x - viewport_offset.x;

	// update text dims
	float inner_height = 0;
	float line_height = 0;
	{
		auto& lines = cache.lines;
		cache.line_y0.clear();
		cache.line_height.clear();
		cache.line_width.clear();
		float y0 = _area.y1 + margin_y - viewport_offset.y;
		cache.content_size = {0,0};
		for (const string &l: lines) {
			auto dim = get_cached_text_dimensions(l, face, font_size, p->ui_scale);
			inner_height = dim.inner_height() / ui_scale;
			line_height = dim.bounding_height / ui_scale * line_height_scale;
			cache.line_height.add(line_height);
			cache.line_y0.add(y0);
			cache.line_width.add(dim.bounding_width / ui_scale);
			cache.content_size.x = max(cache.content_size.x, dim.bounding_width / ui_scale);
			y0 += line_height;
			cache.content_size.y += line_height;
		}
		if (!multiline)
			cache.line_y0[0] = _area.center().y - line_height / 2;
	}

	// selection
	if (cursor_pos != selection_start) {
		p->set_color(color(0.4f, 0.2f, 0.2f, 1.0f));
		auto a = cursor_pos;
		auto b = selection_start;
		if (a > b)
			std::swap(a, b);
		const auto lp0 = index_to_line_pos(a);
		const auto lp1 = index_to_line_pos(b);
		for (int l=lp0.line; l<=lp1.line; l++) {
			float x0 = (l == lp0.line) ? index_to_xy(a).x : text_x0;
			float x1 = (l == lp1.line) ? index_to_xy(b).x : text_x0 + cache.line_width[l];
			p->draw_rect({x0, x1, cache.line_y0[l], cache.line_y0[l] + cache.line_height[0]});
		}
		if (lp1.line == lp0.line) {
			const vec2 pos0 = index_to_xy(a);
			const vec2 pos1 = index_to_xy(b);
			p->draw_rect({pos0.x, pos1.x, pos0.y, pos0.y + cache.line_height[0]});
		}
	}


	// text
	color col0 = Theme::_default.text_label;
	if (!enabled)
		col0 = Theme::_default.text_disabled;
	p->set_color(col0);
	float dy = (line_height - inner_height) / 2;
	for (const auto& [line, l]: enumerate(cache.lines)) {
		if (cache.line_y0[line] + cache.line_height[line] < _area.y1)
			continue;
		if (cache.line_y0[line] > _area.y2)
			continue;
		if (markups.num > 0) {
			int i0 = cache.line_first_index[line];
			int i1 = i0 + l.num;
			float x0 = text_x0;
			for (const auto& m: markups) {
				if (m.i1 >= i0 and m.i0 <= i1) {
					if (m.i0 > i0) {
						// before marker
						p->set_font(font_name, font_size, false, false);
						p->set_color(col0);
						string t = text.sub(i0, m.i0);
						p->draw_str({x0, cache.line_y0[line] + dy}, t);
						x0 += p->get_str_width(t);
						i0 = m.i0;
					}

					{
						// marker
						p->set_color(m.col);
						p->set_font(font_name, font_size, m.flags & FontFlags::Bold, m.flags & FontFlags::Italic);
						string t = text.sub(i0, min(m.i1, i1));
						p->draw_str({x0, cache.line_y0[line] + dy}, t);
						x0 += p->get_str_width(t);
						i0 = min(m.i1, i1);
					}
				}
			}

			if (i0 < i1) {
				// after markers
				p->set_color(col0);
				p->set_font(font_name, font_size, false, false);
				string t = text.sub(i0, i1);
				p->draw_str({x0, cache.line_y0[line] + dy}, t);
			}

		} else {
			p->draw_str({text_x0, cache.line_y0[line] + dy}, l);
		}
	}

	// cursor
	if (has_focus() and enabled) {
		const vec2 pos = index_to_xy(cursor_pos);
		p->draw_line({pos.x, pos.y}, {pos.x, pos.y + cache.line_height[0]});
	}

	p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
	p->set_clip(clip0);

#ifdef PERF_OUT
	float t = timer.get();
	msg_write(f2s(t * 1000, 2));
#endif
}

string Edit::get_range(Index _i0, Index _i1) const {
	auto i0 = min(_i0, _i1);
	auto i1 = max(_i0, _i1);
	return text.sub(i0, i1);
}

void Edit::delete_range(Index i0, Index i1) {
	replace_range(i0, i1, "");
}

void Edit::delete_selection() {
	delete_range(selection_start, cursor_pos);
}

// i0 <= i1
void Edit::_replace_range(Index i0, Index i1, const string& t) {
	clean_markup(i0, i1);
	for (auto& m: markups)
		if (m.i0 >= i1) {
			m.i0 += (i0 - i1) + t.num;
			m.i1 += (i0 - i1) + t.num;
		}

	text = text.sub_ref(0, i0) + t + text.sub_ref(i1);
	cache.rebuild(text);
	auto map_index = [i0, i1, &t] (int index) {
		if (index >= i1)
			return index - (i1 - i0) + t.num;
		if (index >= i0)
			return i0 + t.num;
		return index;
	};
	cursor_pos = map_index(cursor_pos);
	selection_start = map_index(selection_start);
	on_edit();
	emit_event(event_id::Changed, true);
}

void Edit::clear_history() {
	history.clear();
	current_history_index = 0;
}


void Edit::replace_range(Index _i0, Index _i1, const string& t) {
	auto i0 = min(_i0, _i1);
	auto i1 = max(_i0, _i1);
	string old = get_range(i0, i1);
	history.resize(current_history_index);
	history.add({i0, i0 + t.num, old});
	current_history_index ++;
	_replace_range(i0, i1, t);
}

void Edit::auto_insert(const string& t) {
	replace_range(selection_start, cursor_pos, t);
}

void Edit::set_cursor_pos(Index index, bool selecting) {
	cursor_pos = index;
	if (!selecting)
		selection_start = index;
	scroll_into_view(cursor_pos);
	request_redraw();
}

void Edit::scroll_into_view(Index index) {
	if (!face)
		return;
	const auto xy = index_to_xy(index);
	if (xy.x < _area.x1)
		viewport_offset.x -= (_area.x1 - xy.x);
	else if (xy.x > _area.x2)
		viewport_offset.x += (xy.x - _area.x2);
	if (xy.y < _area.y1)
		viewport_offset.y -= (_area.y1 - xy.y);
	else if (xy.y + font_size > _area.y2)
		viewport_offset.y += (xy.y - _area.y2) + font_size * 2;
	viewport_offset = vec2::max(vec2::min(viewport_offset, viewport_size()), vec2::ZERO);
}


vec2 Edit::index_to_xy(Index index) const {
	if (!face)
		return {0,0};
	auto lp = index_to_line_pos(index);
	int first = cache.line_first_index[lp.line];
	face->set_size(font_size * ui_scale);
	auto dim = face->get_text_dimensions(text.sub_ref(first, index));
	float x = text_x0 + dim.bounding_width / ui_scale;
	float y0 = cache.line_y0[lp.line];
	return {x, y0};
}

Edit::Index Edit::xy_to_index(const vec2& pos) const {
	float dx = pos.x - text_x0;
	float dy = pos.y - cache.line_y0[0];

	face->set_size(font_size * ui_scale);
	int line_no = min((int)(dy / cache.line_height[0]), cache.lines.num - 1);
	const auto& l = cache.lines[line_no];

	if (dx > cache.line_width[line_no])
		return line_pos_to_index({line_no, cache.line_num_characters[line_no]});

	int best_i = 0;
	float best_dx = 10000000;
	for (int i=0; i<l.num; i=next_utf8_index(l, i)) {
		auto dim = face->get_text_dimensions(l.sub_ref(0, i));
		float ddx = fabs(dim.bounding_width / ui_scale - dx);
		if (ddx < best_dx) {
			best_dx = ddx;
			best_i = i;
		}
	}
	return line_pos_to_index({line_no, best_i});
}



void Edit::_draw(Painter *p) {
	ui_scale = p->ui_scale;

	// background
	color bg = Theme::_default.background_button;
	if (alt_background)
		bg = Theme::_default.background_low;
	p->set_color(bg);
	p->set_roundness(Theme::_default.button_radius);
	p->draw_rect(_area);

	// focus frame
	if (has_focus() and show_focus_frame) {
		p->set_color(Theme::_default.background_button_primary.with_alpha(0.6f));
		p->draw_rect(_area);

		float dr = Theme::_default.focus_frame_width;
		p->set_roundness(Theme::_default.button_radius - dr);
		p->set_color(bg);
		p->draw_rect(_area.grow(-dr));
	}
	p->set_line_width(1);
	p->set_roundness(0);

	draw_text(p);
}

// TODO count utf8 chars
Edit::LinePos Edit::index_to_line_pos(Index index) const {
	LinePos r = {0, 0};
	for (const auto& [line, first]: enumerate(cache.line_first_index)) {
		if (index < first)
			break;
		r = {line, index - first};
	}
	return r;
}

// TODO count utf8 chars
Edit::Index Edit::line_pos_to_index(const LinePos& lp) const {
	if (lp.line < 0)
		return 0;
	if (lp.line >= cache.lines.num)
		return text.num;
	return cache.line_first_index[lp.line] + clamp(lp.offset, 0, cache.line_num_characters[lp.line]);
}

Edit::Index Edit::next_index(Index index) const {
	return next_utf8_index(text, index);
}

Edit::Index Edit::prior_index(Index index) const {
	return prior_utf8_index(text, index);
}

void Edit::add_markup(const Markup& m) {
	markups.add(m);
	base::inplace_sort(markups, [] (const Markup& a, const Markup& b) {
		return a.i0 <= b.i0;
	});
	request_redraw();
}

void Edit::clean_markup(Index i0, Index i1) {
	// markup completely covered? -> remove
	base::remove_if(markups, [i0, i1] (const Markup& m) {
		return i0 <= m.i0 and i1 >= m.i1;
	});
	// shink?
	for (auto& m: markups) {
		if (m.i0 >= i0 and m.i0 <= i1)
			m.i0 = i1;
		if (m.i1 >= i0 and m.i1 <= i1)
			m.i1 = i0;
	}
	// middle of markup?
	Array<Markup> to_add;
	for (auto& m: markups)
		if (i0 > m.i0 and i1 < m.i1) {
			to_add.add({i1, m.i1, m.flags, m.col});
			m.i1 = i0;
		}
	for (const auto& m: to_add)
		add_markup(m);
	request_redraw();
}



void Edit::set_option(const string& key, const string& value) {
	if (key == "focusframe") {
		show_focus_frame = value._bool();
		request_redraw();
	} else if (key == "font") {
		font_name = value;
		request_redraw();
	} else if (key == "monospace") {
		font_name = "monospace";
		request_redraw();
	} else if (key == "fontsize") {
		font_size = value._float();
		request_redraw();
	} else if (key == "lineheightscale") {
		line_height_scale = value._float();
		request_redraw();
	} else if (key == "altbg") {
		alt_background = true;
	} else {
		Control::set_option(key, value);
	}
}


}
