#pragma once

#include "Control.h"

namespace xhui {

class Edit : public Control {
public:
	Edit(const string& id, const string& title);

	vec2 get_content_min_size() const override;

	void set_string(const string& s) override;
	void enable(bool enabled) override;
	string get_string() override;

	//void on_mouse_enter() override;
	//void on_mouse_leave() override;
	//void on_left_button_down() override;
	//void on_left_button_up() override;
	void on_key_down(int key) override;

	void _draw(Painter* p) override;
	void draw_background(Painter* p);
	void draw_text(Painter* p);
	void draw_active_marker(Painter* p);

	bool multiline = false;
	bool numerical = false;
	string text;
	int cursor_pos = 0;
	bool enabled = true;
	struct Cache {
		Array<string> lines;
		Array<int> line_first_index;
		Array<int> line_num_characters;
		Array<float> line_height;
		Array<float> line_y0;
		Array<float> line_width;

		void rebuild(const string& text);
	} cache;

	struct LinePos {
		int line, offset;
	};

	LinePos index_to_line_pos(int index) const;
	int line_pos_to_index(const LinePos& lp) const;

	// override in SpinButton etc
	virtual void on_edit() {}
};

}
