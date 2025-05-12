#pragma once

#include <functional>

#include "Label.h"
#include "Viewport.h"

namespace xhui {

class Grid;

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
	Array<Control*> get_children(ChildFilter f) const override;

	void _update_selection(const Array<int>& sel);

	vec2 get_content_min_size() const override;
	void negotiate_area(const rect& available) override;

	void on_mouse_enter(const vec2& m) override;
	void on_mouse_leave(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_wheel(const vec2& d) override;
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;
	void on_left_double_click(const vec2& m) override;
	void on_right_button_down(const vec2& m) override;
	void on_right_button_up(const vec2& m) override;
	void on_key_down(int key) override;

	void _draw(Painter *p) override;
	rect row_area(int row) const;
	int get_hover(const vec2& m) const;

	virtual void on_click_row(int row);
	virtual void on_double_click_row(int row);

	Array<string> headers;
	mutable Array<float> column_widths;
	mutable Array<float> column_offsets;
	struct ColumnFactory {
		std::function<Control*(const string& id)> f_create;
		std::function<void(Control* c, const string& t)> f_set;
		std::function<void(Control* c, bool selected)> f_select;
	};
	Array<ColumnFactory> column_factories;
	struct Cell {
		string text;
		Control* control;
	};
	Array<Array<Cell>> cells;
	Viewport viewport;
	Grid* cell_grid;

	enum class SelectionMode {
		SingleOrNone,
		Single,
		Multi
	} selection_mode;

	Array<int> selected;
	//float view_y = 0;
	bool show_headers = true;
	rect padding;
	float selection_radius;

	bool show_selection = true;
	bool sunken_background = true;

	int hover_row = -1;

	string drag_source_id;
};

}
