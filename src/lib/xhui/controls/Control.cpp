#include "Control.h"
#include "../Painter.h"
#include "../../base/algo.h"
#include "../../os/msg.h"

namespace xhui {


rect smaller_rect(const rect& r, float d) {
	return rect(r.x1 + d, r.x2 - d, r.y1 + d, r.y2 - d);
}

Control::Control(const string &_id) {
	id = _id;
	min_width_user = -1;
	min_height_user = -1;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Expand;
	_area = rect::EMPTY;
}

Control::~Control() = default;


void Control::_register(Panel* _owner) {
	// don't register sub-panels!
	if (dynamic_cast<Panel*>(this))
		return;

	//msg_write("REG  " + id + "   ->   " + _owner->id);

	if (owner) {
		/*msg_error("trying to register a control twice  " + id);
		msg_write(p2s(owner));
		msg_write(p2s(_owner));*/
		return;
	}
	owner = _owner;
	if (owner) {
		owner->controls.add(this);
		for (auto cc: get_children(ChildFilter::All))
			cc->_register(owner);
	}
	request_redraw();
}

void Control::_unregister() {
	if (!owner)
		return;
	// don't register sub-panels!
	if (dynamic_cast<Panel*>(this))
		return;
	base::remove(owner->controls, this);
	owner = nullptr;
	for (auto cc: get_children(ChildFilter::All))
		cc->_unregister();
}

Array<Control*> Control::get_children_recursive(bool include_me, ChildFilter f) const {
	Array<Control*> r;
	if (include_me)
		r.add(const_cast<Control*>(this));
	for (auto c: get_children(f))
		r.append(c->get_children_recursive(true, f));
	return r;
}




void Control::request_redraw() {
	if (owner) {
		if (auto w = owner->get_window()) {
			w->redraw(id);
			return;
		}
	} else if (auto w = dynamic_cast<Window*>(this)) {
		w->redraw(id);
	}
	//msg_write(id + "  can not refresh  " + p2s(owner));
}


void Control::get_content_min_size(int &w, int &h) const {
	w = 0;
	h = 0;
}

void Control::get_greed_factor(float &x, float &y) const {
	x = y = 0;
	if (size_mode_x == SizeMode::Expand)
		x = 1;
	if (size_mode_y == SizeMode::Expand)
		y = 1;
}

void Control::get_effective_min_size(int &w, int &h) const {
	get_content_min_size(w, h);
	if (min_width_user >= 0)
		w = min_width_user;
	if (min_height_user >= 0)
		h = min_height_user;
}

void Control::negotiate_area(const rect &available) {
	_area = available;
}

bool Control::has_focus() const {
	if (!owner)
		return false;
	if (auto w = owner->get_window())
		return w->focus_control == this;
	return false;
}

void Control::on_mouse_move(const vec2& m, const vec2& d) {
	emit_event(event_id::MouseMove, false);
}

void Control::emit_event(const string& msg, bool is_default) {
	if (owner)
		owner->handle_event(id, msg, is_default);
}

void Control::set_option(const string& key, const string& value) {
	if (key == "expandx") {
		size_mode_x = SizeMode::Expand;
		if (value._bool())
			size_mode_x = SizeMode::Shrink;
		request_redraw();
	} else if (key == "expandy") {
		size_mode_y = SizeMode::Expand;
		if (value._bool())
			size_mode_y = SizeMode::Shrink;
		request_redraw();
	} else if (key == "noexpandx") {
		size_mode_x = SizeMode::Shrink;
		request_redraw();
	} else if (key == "noexpandy") {
		size_mode_y = SizeMode::Shrink;
		request_redraw();
	} else if (key == "width") {
		min_width_user = value._int();
		request_redraw();
	} else if (key == "height") {
		min_height_user = value._int();
		request_redraw();
	} else if (key == "ignorehover") {
		ignore_hover = true;
	} else if (key == "ignorefocus") {
		can_grab_focus = false;
	} else if (key == "cangrabfocus") {
		can_grab_focus = value._bool() or (value == "");
	} else if (key == "grabfocus") {
		can_grab_focus = true;
		run_later(0.01f, [this] {
			if (owner)
				if (auto w = owner->get_window())
					w->focus_control = this;
		});
	} else if (key == "visible") {
		visible = value._bool() or value == "";
	}
}




}
