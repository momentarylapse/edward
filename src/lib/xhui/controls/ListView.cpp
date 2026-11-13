#include "ListView.h"
#include "Grid.h"
#include "../Painter.h"
#include "../Theme.h"
#include <lib/base/iter.h>

namespace xhui {

static constexpr float HEADER_DY = 30;


base::set<int> create_range_set(int from, int to) {
	base::set<int> r;
	for (int i=min(from, to); i<=max(from, to); i++)
		r.add(i);
	return r;
}

ListView::ListView(const string &_id, const string &t) :
		Control(_id),
		viewport(_id + ":viewport")
{
	can_grab_focus = true;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Expand;
	headers = t.explode("\\");
	column_widths.resize(headers.num);
	column_offsets.resize(headers.num);
	for (int i=0; i<headers.num; i++) {
		column_factories.add({[this] (const string& id) {
			return create_control("Label", format("!paddingx=%.1f,paddingy=%.1f", cell_padding.x, cell_padding.y), id);
		}, [] (Control* c, const string& t) {
			c->set_string(t);
		}});
	}
	cell_grid = new Grid(_id + ":grid");
	cell_grid->padding = {0,0,0,0};//{7,7,4,4};
	cell_grid->grid.spacing = 4;
	cell_padding = {4, 4};
	viewport.add_child(cell_grid, 0, 0);
	viewport.size_mode_x = SizeMode::Fill;
	viewport.size_mode_y = SizeMode::Fill;
	viewport.ignore_hover = true;
	padding = {5, 5, 5, 5};
	selection_radius = Theme::_default.button_radius;
	selection_mode = SelectionMode::SingleOrNone;
}

void ListView::on_left_button_down(const vec2& m) {
	owner->get_window()->start_pre_drag(this);
	hover_row = get_hover(m);

#ifdef OS_MAC
	constexpr int mod = KEY_SUPER;
#else
	constexpr int mod = KEY_CONTROL;
#endif

	if (get_window()->is_key_pressed(mod)) {
		if (hover_row >= 0) {
			auto new_sel = selected;
			if (selected.contains(hover_row))
				new_sel.erase(hover_row);
			else
				new_sel.add(hover_row);
			_update_selection(new_sel);
			cursor_begin = cursor = hover_row;
		}
	} else if (get_window()->is_key_pressed(KEY_SHIFT)) {
		if (hover_row >= 0) {
			cursor = hover_row;
			_update_selection(create_range_set(cursor_begin, cursor));
		}
	} else {
		set_selection({hover_row});
	}
}

void ListView::_update_selection(const base::set<int>& sel) {
	if (sel.num == 0 and selection_mode == SelectionMode::Single)
		return;

	selected = sel;

	if (column_factories[0].f_select)
		for (const auto& [i,r]: enumerate(cells))
			column_factories[0].f_select(r[0].control.get(), selected.contains(i));

	request_redraw();
	emit_event(event_id::Select, false);
}

void ListView::on_click_row(int row) {
	emit_event(event_id::Click, false);
}

void ListView::on_double_click_row(int row) {
	if (!emit_event(event_id::Activate, true))
		emit_event(event_id::ActivateDialogDefault, false);
}

void ListView::on_left_button_up(const vec2&) {
	request_redraw();
	if (hover_row >= 0)
		on_click_row(hover_row);
}

void ListView::on_left_double_click(const vec2& m) {
	if (hover_row >= 0)
		on_double_click_row(hover_row);
}

void ListView::on_right_button_down(const vec2& m) {
	hover_row = get_hover(m);
	set_selection({hover_row});
}

void ListView::on_right_button_up(const vec2&) {
	request_redraw();
	emit_event(event_id::RightButtonUp, false);
}

void ListView::on_mouse_enter(const vec2&) {
	//state = State::HOVER;
	request_redraw();
}
void ListView::on_mouse_leave(const vec2&) {
	//state = State::DEFAULT;
	hover_row = -1;
	request_redraw();
}
void ListView::on_mouse_move(const vec2& m, const vec2& d) {
	if (!owner->get_window()->button(0))
		hover_row = get_hover(m);
	request_redraw();
}

void ListView::on_mouse_wheel(const vec2& d) {
	viewport.on_mouse_wheel(d);
}

void ListView::on_key_down(int key) {
#ifdef OS_MAC
	constexpr int key_begin = KEY_UP + KEY_SUPER;
	constexpr int key_end = KEY_DOWN + KEY_SUPER;
#else
	constexpr int key_begin = KEY_HOME;
	constexpr int key_end = KEY_END;
#endif

	if (key == KEY_RETURN) {
		if (selected.num > 0)
			if (!emit_event(event_id::Activate, true))
				emit_event(event_id::ActivateDialogDefault, false);
	} else if (key == KEY_DOWN) {
		if (cursor >= 0)
			set_selection({min(cursor + 1, cells.num - 1)});
		else if (cells.num >= 0)
			set_selection({0});
	} else if (key == KEY_UP) {
		if (cursor >= 0)
			set_selection({max(cursor - 1, 0)});
		else if (cells.num >= 0)
			set_selection({0});
	} else if (key == key_begin) {
		if (cells.num > 0)
			set_selection({0});
	} else if (key == key_end) {
		if (cells.num > 0)
			set_selection({cells.num - 1});
	} else if (key == KEY_DOWN + KEY_SHIFT) {
		if (cursor >= 0) {
			cursor = min(cursor + 1, cells.num - 1);
			_update_selection(create_range_set(cursor_begin, cursor));
		} else if (cells.num >= 0) {
			set_selection({0});
		}
	} else if (key == KEY_UP + KEY_SHIFT) {
		if (cursor >= 0) {
			cursor = max(cursor - 1, 0);
			_update_selection(create_range_set(cursor_begin, cursor));
		} else if (cells.num >= 0) {
			set_selection({0});
		}
	} else if (key == key_begin + KEY_SHIFT) {
		if (cells.num > 0 and cursor >= 0) {
			cursor = 0;
			_update_selection(create_range_set(cursor_begin, cursor));
		}
	} else if (key == key_end + KEY_SHIFT) {
		if (cells.num > 0 and cursor >= 0) {
			cursor = cells.num - 1;
			_update_selection(create_range_set(cursor_begin, cursor));
		}
	}
}


int ListView::get_hover(const vec2& m) const {
	for (int i=0; i<cells.num; i++)
		if (row_area(i).inside(m))
			return i;
	return -1;
}



vec2 ListView::get_content_min_size() const {
	vec2 s = viewport.effective_min_size();
	if (show_headers)
		s.y += HEADER_DY;
	return s;
}

void ListView::negotiate_content_area(const rect& available) {
	float dy = 0;
	if (show_headers)
		dy = HEADER_DY;
	viewport.negotiate_outer_area({available.p00() + vec2(0, dy), available.p11()});
	if (show_headers and cells.num > 0) {
		for (int i=0; i<min(headers.num, cells[0].num); i++)
			column_offsets[i] = cells[0][i].control->area.x1 - area.x1;
	}
}


rect ListView::row_area(int row) const {
	const auto r0 = cells[row][0].control->outer_area();
	return {area.x1, area.x2, r0.y1, r0.y2};
}


void ListView::_draw(Painter *p) {
	if (sunken_background) {
		color bg = Theme::_default.background_low;
		p->set_color(bg);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(area);
		p->set_roundness(0);
	}

	p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
	//auto dim = font::get_text_dimensions(title);


	if (show_headers) {
		p->set_color(Theme::_default.text_disabled);
		for (int col=0; col<headers.num; col++) {
			p->draw_str({area.x1 + (float)column_offsets[col], area.y1 + 9}, headers[col]);
		}
	}

	if (show_selection) {
		p->set_roundness(selection_radius);
		if (hover_row >= 0) {
			p->set_color(Theme::_default.background_hover.with_alpha(0.5f));
			p->draw_rect(row_area(hover_row) and viewport.area);
		}
		for (int row: selected) {
			p->set_color(Theme::_default.background_low_selected);
			p->draw_rect(row_area(row) and viewport.area);
		}
		p->set_roundness(0);
	}

	viewport._draw(p);
}

void ListView::add_string(const string& s) {
	int row = cells.num;
	cells.add({});
	auto xx = s.explode("\\");
	while (xx.num < column_factories.num)
		xx.add("");
	for (const auto& [col, t]: enumerate(xx)) { //s.explode("\\"))) {
		if (col >= column_factories.num)
			continue;
		string cid = format("%s:%d:%d", id, row, col);
		auto c = column_factories[col].f_create(cid);
		if (col == column_factories.num - 1)
			c->size_mode_x = SizeMode::Fill;
		if (auto p = as_panel(c))
			owner->embed(cell_grid->id, col, row, p);
		else
			cell_grid->add_child(c, col, row);
		cells.back().add({t, c});
		column_factories[col].f_set(c, t);
	}
	if (cells.num == 1 and selection_mode == SelectionMode::Single)
		set_selection({0});
	request_redraw();
}

void ListView::set_cell(int row, int col, const string& s) {
	if (row >= 0 and row < cells.num)
		if (col >= 0 and col < cells[row].num and col < column_factories.num) {
			cells[row][col].text = s;
			column_factories[col].f_set(cells[row][col].control.get(), s);
		}
	request_redraw();
}

void ListView::reset() {
	hover_row = -1;
	cursor_begin = cursor = -1;
	selected.clear();
	for (auto c: cell_grid->get_children(ChildFilter::All)) {
		if (auto p = as_panel(c))
			owner->unembed(p);
		else
			cell_grid->remove_child(c);
	}
	cells.clear();
	viewport.offset = vec2::ZERO;
	request_redraw();
}

void ListView::set_int(int i) {
	set_selection({i});
}

void ListView::set_selection(const Array<int>& sel) {
	base::set<int> _sel;
	for (int i: sel)
		if (i >= 0 and i < cells.num)
			_sel.add(i);
	_update_selection(_sel);

	if (_sel.num == 1)
		cursor_begin = cursor = _sel[0];
	if (_sel.num == 0 and selection_mode != SelectionMode::Single)
		cursor_begin = cursor = -1;
}

string ListView::get_cell(int row, int col) {
	if (row >= 0 and row < cells.num)
		if (col >= 0 and col < cells[row].num)
			return cells[row][col].text;
	return "";
}

int ListView::get_int() {
	if (selected.num > 0)
		return selected[0];
	return -1;
}

Array<int> ListView::get_selection() {
	return selected;
}

void ListView::set_option(const string& key, const string& value) {
	if (key == "bar") {
		show_headers = value._bool();
	} else if (key == "nobar") {
		show_headers = false;
	} else if (key == "showselection") {
		show_selection = value._bool();
	} else if (key == "sunkenbackground") {
		sunken_background = value._bool();
		if (!sunken_background) {
			padding = {0,0,0,0};
			cell_grid->padding = {0, 0, 0, 0};
		}
	} else if (key == "style") {
		if (value == "compact") {
			padding = {0,0,0,0};
			selection_radius = 0;
			cell_padding = {4, 4};
		} else if (value == "larger") {
			padding = {5, 5, 5, 5};
			selection_radius = Theme::_default.button_radius;
			cell_padding = {6, 9};
		}
	} else if (key == "spacing") {
		cell_grid->grid.spacing = value._float();
	} else if (key == "dragsource") {
		drag_source_id = value;
	} else if (key == "selectsingle" or key == "select-single") {
		selection_mode = SelectionMode::Single;
	} else if (key == "selectmulti" or key == "select-multi") {
		selection_mode = SelectionMode::Multi;
	} else {
		Control::set_option(key, value);
	}

	request_redraw();
}

Array<const layout::Node*> ListView::_get_children(ChildFilter f) const {
	return {&viewport};
}


}
