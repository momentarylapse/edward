#pragma once

#include "../../base/base.h"
#include "../../math/rect.h"
#include "../../math/vec2.h"

namespace xhui {

class Painter;
class Window;
class Panel;

rect smaller_rect(const rect& r, float d);

class Control : public VirtualBase {
	friend class Window;
	friend class Panel;
public:
	explicit Control(const string& id);

	virtual void set_string(const string& s) {}
	virtual void add_string(const string& s) { set_string(s); }
	virtual void set_cell(int row, int col, const string& s) {}
	virtual void reset() {}
	virtual void set_int(int i) { set_string(str(i)); }
	virtual string get_string() { return ""; }
	virtual string get_cell(int row, int col) { return ""; }
	virtual int get_int() { return get_string()._int(); }
	virtual Array<int> get_selection() { return {}; }
	virtual void enable(bool enabled) {}
	virtual void set_option(const string& key, const string& value) {}
	virtual Array<Control*> get_children() const { return {}; }

	virtual void on_left_button_down(const vec2& m) {}
	virtual void on_left_button_up(const vec2& m) {}
	virtual void on_middle_button_down(const vec2& m) {}
	virtual void on_middle_button_up(const vec2& m) {}
	virtual void on_right_button_down(const vec2& m) {}
	virtual void on_right_button_up(const vec2& m) {}
	virtual void on_mouse_move(const vec2& m, const vec2& d);
	virtual void on_mouse_enter(const vec2& m) {}
	virtual void on_mouse_leave(const vec2& m) {}
	virtual void on_mouse_wheel(const vec2& d) {}
	virtual void on_key_down(int key) {}
	virtual void on_key_up(int key) {}

	virtual void _draw(Painter *p) {}

	void request_redraw();
	bool has_focus() const;

	rect _area;
	string id;
	Panel *owner = nullptr;

	int min_width_user, min_height_user;
	bool expand_x, expand_y;
	bool can_grab_focus = false;
	bool ignore_hover = false;
	bool visible = true;

	virtual void get_greed_factor(float &x, float &y);
	virtual void get_content_min_size(int &w, int &h);
	void get_effective_min_size(int &w, int &h);

	//virtual void negotiate_min_size();
	virtual void negotiate_area(const rect &available);


	enum class State {
		DEFAULT,
		HOVER,
		PRESSED,
		DISABLED
	};
};

}
