#pragma once

#include <lib/base/optional.h>
#include <lib/ygraphics/font.h>

#include "Control.h"

namespace xhui {

enum class FontFlags {
	None = 0,
	Bold = 1,
	Italic = 2,
	Underline = 4
};

FontFlags operator|(FontFlags a, FontFlags b);
bool operator&(FontFlags a, FontFlags b);

class Edit : public Control {
public:
	Edit(const string& id, const string& title);

	vec2 get_content_min_size() const override;

	void set_string(const string& s) override;
	void enable(bool enabled) override;
	string get_string() override;
	void set_option(const string& key, const string& value) override;

	//void on_mouse_enter() override;
	//void on_mouse_leave() override;
	void on_left_button_down(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_wheel(const vec2& d) override;
	//void on_left_button_up() override;
	void on_key_down(int key) override;
	void on_key_char(int character) override;

	void _draw(Painter* p) override;
	void draw_background(Painter* p);
	void draw_text(Painter* p);
	void draw_active_marker(Painter* p);

	// byte offset in text buffer
	using Index = int;

	void set_cursor_pos(Index index, bool selecting = false);
	void scroll_into_view(Index index);

	mutable float ui_scale = 1.0f;
	bool multiline = false;
	bool numerical = false;
	bool show_focus_frame = true;
	bool show_line_numbers = false;
	float line_number_area_width = 0;
	float margin_x, margin_y;
	string font_name;
	float font_size;
	float line_height_scale = 1.0f;
	int tab_size;
	font::Face* face;
	bool alt_background = false;
	string text;
	Index cursor_pos = 0;
	Index selection_start = 0;
	vec2 viewport_offset = {0,0};
	vec2 viewport_size() const;
	bool enabled = true;
	float text_x0 = 0;
	struct Cache {
		Array<string> lines;
		Array<Index> line_first_index;
		Array<int> line_num_characters;
		Array<float> line_height;
		Array<float> line_y0;
		Array<float> line_width;
		vec2 content_size;

		void rebuild(const string& text);
	} cache;

	void delete_range(Index i0, Index i1);
	void delete_selection();
	void _replace_range(Index i0, Index i1, const string& t);
	void replace_range(Index i0, Index i1, const string& t);
	void auto_insert(const string& t);
	void multi_line_indent(int indent);
	string get_range(Index i0, Index i1) const;

	struct LinePos {
		int line, offset;
	};

	LinePos index_to_line_pos(Index index) const;
	Index line_pos_to_index(const LinePos& lp) const;
	Index prior_index(Index index) const;
	Index next_index(Index index) const;
	vec2 index_to_xy(Index index) const;
	Index xy_to_index(const vec2& pos) const;

	struct Markup {
		Index i0 = 0, i1 = 0;
		FontFlags flags = FontFlags::None;
		color col;
	};
	Array<Markup> markups;
	bool markup_dirty = false;
	void add_markup(const Markup& m);
	void clean_markup(Index i0, Index i1);

	struct Operation {
		Index i0, i1;
		string t;
	};
	Array<Operation> history;
	int current_history_index = 0;
	void clear_history();


	// override in SpinButton etc
	virtual void on_edit() {}
};

}
