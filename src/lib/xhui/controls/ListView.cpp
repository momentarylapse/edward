#include "ListView.h"
#include "../Painter.h"
#include "../draw/font.h"
#include "../Theme.h"

namespace xhui {

	ListView::ListView(const string &_id, const string &t) : Control(_id) {
		can_grab_focus = true;
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Expand;
		headers = t.explode("\\");
		column_widths.resize(headers.num);
		column_offsets.resize(headers.num);
	}

	void ListView::on_left_button_down(const vec2& m) {
		owner->get_window()->start_pre_drag(this);
		hover_row = get_hover(m);
		selected = {};
		if (hover_row >= 0)
			selected = {hover_row};
		request_redraw();
		emit_event(event_id::Select, false);
	}
	void ListView::on_left_button_up(const vec2&) {
		request_redraw();

		emit_event(event_id::Click, true);
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

	int ListView::get_hover(const vec2& m) const {
		for (int i=0; i<cells.num; i++)
			if (row_area(i).inside(m))
				return i;
		return -1;
	}



	void ListView::get_content_min_size(int &w, int &h) {
		column_widths.resize(headers.num);
		column_offsets.resize(headers.num);
		w = 5;
		for (int col=0; col<headers.num; col++) {
			column_widths[col] = 120;
			column_offsets[col] = w;
			w += column_widths[col];
		}
		h = 10;
	}

	rect ListView::row_area(int row) const {
		float dy = 0;
		if (show_headers)
			dy = 30;
		return {_area.p00() + vec2(0, 30 * (float)row + dy), _area.p10() + vec2(0, 30 * ((float)row + 1) + dy)};
	}


	void ListView::_draw(Painter *p) {
		color bg = Theme::_default.background_low;
		p->set_color(bg);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(_area);
		p->set_roundness(0);

		p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
		//auto dim = font::get_text_dimensions(title);

		if (show_headers) {
			p->set_color(Theme::_default.text_disabled);
			for (int col=0; col<headers.num; col++) {
				p->draw_str({_area.x1 + (float)column_offsets[col], _area.y1 + 9}, headers[col]);
			}
		}

		if (hover_row >= 0) {
			p->set_color(Theme::_default.background_hover);
	//		p->set_roundness(Theme::_default.button_radius);
			p->draw_rect(row_area(hover_row));
			p->set_roundness(0);
		}
		for (int row: selected) {
			p->set_color(Theme::_default.background_low_selected);
	//		p->set_roundness(Theme::_default.button_radius);
			p->draw_rect(row_area(row));
			p->set_roundness(0);
		}

		p->set_color(Theme::_default.text);
		for (int row=0; row<cells.num; row++) {
			for (int col=0; col<cells[row].num; col++) {
				p->draw_str(row_area(row).p00() + vec2((float)column_offsets[col], 9), cells[row][col]);
			}
		}
	}

	void ListView::add_string(const string& s) {
		cells.add(s.explode("\\"));
		request_redraw();
	}
	void ListView::set_cell(int row, int col, const string& s) {
		if (row >= 0 and row < cells.num)
			if (col >= 0 and col < cells[row].num)
				cells[row][col] = s;
		request_redraw();
	}
	void ListView::reset() {
		cells.clear();
		request_redraw();
	}
	void ListView::set_int(int i) {
		selected = {i};
		request_redraw();
	}
	string ListView::get_cell(int row, int col) {
		if (row >= 0 and row < cells.num)
			if (col >= 0 and col < cells[row].num)
				return cells[row][col];
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
		} else if (key == "dragsource") {
			drag_source_id = value;
		} else {
			Control::set_option(key, value);
		}

		request_redraw();
	}

}
