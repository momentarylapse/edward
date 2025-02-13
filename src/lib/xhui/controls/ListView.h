#pragma once

#include "Label.h"

namespace xhui {

	class ListView : public Control {
	public:
		ListView(const string &id, const string &title);

		void add_string(const string& s) override;
		void set_cell(int row, int col, const string& s) override;
		void reset() override;
		void set_int(int i) override;
		string get_cell(int row, int col) override;
		int get_int() override;
		Array<int> get_selection() override;
		void set_option(const string& key, const string& value) override;

		void get_content_min_size(int &w, int &h) const override;

		void on_mouse_enter(const vec2& m) override;
		void on_mouse_leave(const vec2& m) override;
		void on_mouse_move(const vec2& m, const vec2& d) override;
		void on_mouse_wheel(const vec2& d) override;
		void on_left_button_down(const vec2& m) override;
		void on_left_button_up(const vec2& m) override;
		void on_left_double_click(const vec2& m) override;
		void on_right_button_down(const vec2& m) override;
		void on_right_button_up(const vec2& m) override;

		void _draw(Painter *p) override;
		rect row_area(int row) const;
		int get_hover(const vec2& m) const;

		Array<string> headers;
		mutable Array<int> column_widths;
		mutable Array<int> column_offsets;
		Array<Array<string>> cells;
		Array<int> selected;
		float view_y = 0;
		bool show_headers = true;

		int hover_row = -1;

		string drag_source_id;
	};

}
