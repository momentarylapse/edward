#include "ListView.h"
#include "Grid.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../../base/iter.h"

namespace xhui {

static constexpr float HEADER_DY = 30;

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
			return new Label(id, "");
		}, [this] (Control* c, const string& t) {
			c->set_string(t);
		}});
	}
	cell_grid = new Grid(_id + ":grid");
	cell_grid->margin = {7,7,4,4};
	cell_grid->spacing = 8;
	viewport.add_child(cell_grid, 0, 0);
	viewport.size_mode_y = SizeMode::Shrink;
	viewport.ignore_hover = true;
	padding = {5, 5, 5, 5};
	selection_radius = Theme::_default.button_radius;
	selection_mode = SelectionMode::SingleOrNone;
}

void ListView::on_left_button_down(const vec2& m) {
	owner->get_window()->start_pre_drag(this);
	hover_row = get_hover(m);
	if (hover_row >= 0)
		_update_selection({hover_row});
	else if (selection_mode != SelectionMode::Single)
		_update_selection({});
}

void ListView::_update_selection(const Array<int>& sel) {
	selected = sel;

	if (column_factories[0].f_select)
		for (const auto& [i,r]: enumerate(cells))
			column_factories[0].f_select(r[0].control, selected.find(i) >= 0);

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
	if (selection_mode != SelectionMode::Single)
		selected = {};
	if (hover_row >= 0)
		selected = {hover_row};
	request_redraw();
	emit_event(event_id::RightButtonDown, false);
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
	if (key == KEY_RETURN) {
		if (selected.num > 0)
			if (!emit_event(event_id::Activate, true))
				emit_event(event_id::ActivateDialogDefault, false);
	} else if (key == KEY_DOWN) {
		if (selected.num > 0)
			_update_selection({min(selected[0] + 1, cells.num - 1)});
	} else if (key == KEY_UP) {
		if (selected.num > 0)
			_update_selection({max(selected[0] - 1, 0)});
	}
}



int ListView::get_hover(const vec2& m) const {
	for (int i=0; i<cells.num; i++)
		if (row_area(i).inside(m))
			return i;
	return -1;
}



vec2 ListView::get_content_min_size() const {
	vec2 s = viewport.get_content_min_size() + padding.p00() + padding.p11();
	if (show_headers)
		s.y += HEADER_DY;
	return s;
}

void ListView::negotiate_area(const rect& available) {
	Control::negotiate_area(available);
	float dy = 0;
	if (show_headers)
		dy = HEADER_DY;
	viewport.negotiate_area({available.p00() + vec2(0, dy) + padding.p00(), available.p11() - padding.p11()});
	if (show_headers and cells.num > 0) {
		for (int i=0; i<min(headers.num, cells[0].num); i++)
			column_offsets[i] = cells[0][i].control->_area.x1 - _area.x1;
	}
}


rect ListView::row_area(int row) const {
	const auto r0 = cells[row][0].control->_area;
	return {_area.x1, _area.x2, r0.y1 - 4, r0.y2 + 4};
}


void ListView::_draw(Painter *p) {
	if (sunken_background) {
		color bg = Theme::_default.background_low;
		p->set_color(bg);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(_area);
		p->set_roundness(0);
	}

	p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
	//auto dim = font::get_text_dimensions(title);


	if (show_headers) {
		p->set_color(Theme::_default.text_disabled);
		for (int col=0; col<headers.num; col++) {
			p->draw_str({_area.x1 + (float)column_offsets[col], _area.y1 + 9}, headers[col]);
		}
	}

	if (show_selection) {
		p->set_roundness(selection_radius);
		if (hover_row >= 0) {
			p->set_color(Theme::_default.background_hover.with_alpha(0.5f));
			p->draw_rect(row_area(hover_row) and viewport._area);
		}
		for (int row: selected) {
			p->set_color(Theme::_default.background_low_selected);
			p->draw_rect(row_area(row) and viewport._area);
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
		//c->size_mode_x = SizeMode::Expand;
		if (auto p = as_panel(c))
			owner->embed(cell_grid->id, col, row, p);
		else
			cell_grid->add_child(c, col, row);
		cells.back().add({t, c});
		column_factories[col].f_set(c, t);
	}
	if (cells.num == 1 and selection_mode == SelectionMode::Single)
		selected = {0};
	request_redraw();
}

void ListView::set_cell(int row, int col, const string& s) {
	if (row >= 0 and row < cells.num)
		if (col >= 0 and col < cells[row].num and col < column_factories.num) {
			cells[row][col].text = s;
			column_factories[col].f_set(cells[row][col].control, s);
		}
	request_redraw();
}

void ListView::reset() {
	hover_row = -1;
	selected.clear();
	for (auto c: cell_grid->get_children(ChildFilter::All))
		cell_grid->remove_child(c);
	cells.clear();
	viewport.offset = vec2::ZERO;
	request_redraw();
}
void ListView::set_int(int i) {
	if (selection_mode != SelectionMode::Single)
		selected = {};
	if (i >= 0 and i < cells.num)
		selected = {i};

	if (column_factories[0].f_select)
		for (const auto& [i,r]: enumerate(cells))
			column_factories[0].f_select(r[0].control, selected.find(i) >= 0);

	request_redraw();
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
			cell_grid->margin = {0, 0, 0, 0};
		}
	} else if (key == "style") {
		if (value == "compact") {
			padding = {0,0,0,0};
			selection_radius = 0;
		}
	} else if (key == "padding") {
		float x = value._float();
		padding = {x, x, x, x};
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

Array<Control*> ListView::get_children(ChildFilter f) const {
	return {(Control*)&viewport};
}


}
