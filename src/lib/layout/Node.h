#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/math/rect.h>
#include <lib/math/vec2.h>

namespace layout {

enum class ChildFilter {
	All,
	OnlyActive
};

enum class Orientation {
	HORIZONTAL,
	VERTICAL,
};

enum class SizeMode {
	Shrink, // as small as possible = get_content_min_size()
	Fill,   // use up available space, >= get_content_min_size()
	Expand, // as large as possible (using greed-factor)
	ForwardChild
};

class Node : public Sharable<VirtualBase> {
public:
	explicit Node(const string& id);
	~Node() override;

	// full registration!
//	virtual void add_child(shared<Node> c, int x, int y) {}
//	virtual void remove_child(Node* c) {}

//	virtual Array<Control*> get_children(ChildFilter f) const { return {}; }
//	Array<Control*> get_children_recursive(bool include_me, ChildFilter f) const;

/*	virtual void on_left_button_down(const vec2& m) {}
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
	virtual void on_key_char(int character) {}*/

	void set_layout_option(const string& key, const string& value);

	string id;
	rect area;

	float min_width_user, min_height_user;
	SizeMode size_mode_x, size_mode_y;
	vec2 greed_factor = vec2(1, 1); // if expanding...
	bool visible = true;

	virtual vec2 get_greed_factor() const;
	virtual vec2 get_content_min_size() const;
	vec2 get_effective_min_size() const;

	//virtual void negotiate_min_size();
	virtual void negotiate_area(const rect &available);
};

}
