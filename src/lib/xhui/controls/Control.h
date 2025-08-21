#pragma once

#include "../../base/base.h"
#include "../../base/pointer.h"
#include "../../math/rect.h"
#include "../../math/vec2.h"
#include "../../image/color.h"

namespace xhui {

class Painter;
class Window;
class Dialog;
class Panel;

enum class ChildFilter {
	All,
	OnlyActive
};

enum class ControlType {
	SomeControl,
	Button,
	Panel,
	Dialog,
	Window
};

enum class Orientation {
	HORIZONTAL,
	VERTICAL,
};

class Control : public Sharable<VirtualBase> {
	friend class Window;
	friend class Panel;
public:
	explicit Control(const string& id, ControlType type = ControlType::SomeControl);
	~Control() override;

	void _register(Panel* owner);
	void _unregister();
	void _unregister_from_window();

	// full registration!
	virtual void add_child(shared<Control> c, int x, int y) {}
	virtual void remove_child(Control* c) {}

	virtual void set_string(const string& s) {}
	virtual void add_string(const string& s) { set_string(s); }
	virtual void set_cell(int row, int col, const string& s) {}
	virtual void reset() {}
	virtual void set_int(int i) { set_string(str(i)); }
	virtual void set_float(float f) { set_string(str(f)); }
	virtual void check(bool checked) {}
	virtual void expand(bool expanded) {}
	virtual void set_color(const color& c) {}
	virtual string get_string() { return ""; }
	virtual string get_cell(int row, int col) { return ""; }
	virtual int get_int() { return get_string()._int(); }
	virtual float get_float() { return get_string()._float(); }
	virtual bool is_checked() { return false; }
	virtual color get_color() { return Black; }
	virtual Array<int> get_selection() { return {}; }
	virtual void enable(bool enabled);
	virtual void set_option(const string& key, const string& value);
	virtual Array<Control*> get_children(ChildFilter f) const { return {}; }
	Array<Control*> get_children_recursive(bool include_me, ChildFilter f) const;

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
	virtual void on_key_char(int character) {}

	virtual void _draw(Painter *p) {}

	void request_redraw();
	bool has_focus() const;

	rect _area;
	string id;
	ControlType type;

	// Control: surrounding Panel (might be Window)
	// Panel: parent Panel
	Panel *owner = nullptr;
	Window* get_window() const;

	enum class SizeMode {
		Shrink,
		Expand,
		ForwardChild
	};

	float min_width_user, min_height_user;
	SizeMode size_mode_x, size_mode_y;
	vec2 greed_factor = vec2(1, 1); // if expanding...
	bool can_grab_focus = false;
	bool ignore_hover = false;
	bool visible = true;
	bool enabled;

	virtual vec2 get_greed_factor() const;
	virtual vec2 get_content_min_size() const;
	vec2 get_effective_min_size() const;

	//virtual void negotiate_min_size();
	virtual void negotiate_area(const rect &available);

	bool emit_event(const string& msg, bool is_default);
};

}
