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

	void ListView::on_left_button_down(const vec2&) {
		request_redraw();
	}
	void ListView::on_left_button_up(const vec2&) {
		request_redraw();

		if (owner)
			owner->handle_event(id, "hui:click", true);
	}
	void ListView::on_mouse_enter(const vec2&) {
		//state = State::HOVER;
		request_redraw();
	}
	void ListView::on_mouse_leave(const vec2&) {
		//state = State::DEFAULT;
		request_redraw();
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

	void ListView::_draw(Painter *p) {
		color bg = Theme::_default.background_low;
		p->set_color(bg);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(_area);
		p->set_roundness(0);

		font::set_font(Theme::_default.font_name, Theme::_default.font_size);
		//auto dim = font::get_text_dimensions(title);

		p->set_color(Theme::_default.text_disabled);
		for (int col=0; col<headers.num; col++) {
			p->draw_str({_area.x1 + (float)column_offsets[col], _area.y1 + 5}, headers[col]);
		}
		p->set_color(Theme::_default.text);
		for (int row=0; row<cells.num; row++) {
			for (int col=0; col<cells[row].num; col++) {
				p->draw_str({_area.x1 + (float)column_offsets[col], _area.y1 + 5 + (row + 1) * 30}, cells[row][col]);
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
		if (key == "bar")
			show_headers = value._bool();
		request_redraw();
	}

}
