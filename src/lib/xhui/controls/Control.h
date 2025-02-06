#pragma once

#include "../../image/color.h"
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

	void _register(Panel* owner);
	void _unregister();

	virtual void add(Control* c, int x, int y) {}
	virtual void set_string(const string& s) {}
	virtual void add_string(const string& s) { set_string(s); }
	virtual void set_cell(int row, int col, const string& s) {}
	virtual void reset() {}
	virtual void set_int(int i) { set_string(str(i)); }
	virtual void set_float(float f) { set_string(str(f)); }
	virtual void set_color(const color& c) {}
	virtual string get_string() { return ""; }
	virtual string get_cell(int row, int col) { return ""; }
	virtual int get_int() { return get_string()._int(); }
	virtual float get_float() { return get_string()._float(); }
	virtual color get_color() { return Black; }
	virtual Array<int> get_selection() { return {}; }
	virtual void enable(bool enabled) {}
	virtual void set_option(const string& key, const string& value);
	virtual Array<Control*> get_children() const { return {}; }
	Array<Control*> get_children_recursive(bool include_me) const;

	virtual void on_left_button_down(const vec2& m) {}
	virtual void on_left_button_up(const vec2& m) {}
	virtual void on_left_double_click(const vec2& m) {}
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

	// Control: surrounding Panel (might be Window)
	// Panel: parent Panel
	Panel *owner = nullptr;

	enum class SizeMode {
		Shrink,
		Expand,
		ForwardChild
	};

	int min_width_user, min_height_user;
	SizeMode size_mode_x, size_mode_y;
	bool can_grab_focus = false;
	bool ignore_hover = false;
	bool visible = true;

	virtual void get_greed_factor(float &x, float &y);
	virtual void get_content_min_size(int &w, int &h);
	void get_effective_min_size(int &w, int &h);

	//virtual void negotiate_min_size();
	virtual void negotiate_area(const rect &available);

	void emit_event(const string& msg, bool is_default);


	enum class State {
		DEFAULT,
		HOVER,
		PRESSED,
		DISABLED
	};
};

}
