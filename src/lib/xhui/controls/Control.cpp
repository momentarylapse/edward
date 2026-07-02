#include "Control.h"
#include "../Painter.h"
#include "../../base/algo.h"
#include "../../os/msg.h"

namespace xhui {



Control::Control(const string &_id, ControlType _type) : Node(_id) {
	type = _type;
	enabled = true;
	align = {0, 0};
}

Control::~Control() = default;


void Control::_register(Panel* _owner) {
	// don't register sub-panels!
	if (type == ControlType::Panel)
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

void Control::_unregister_from_window() {
	if (!owner)
		return;
	if (auto w = owner->get_window()) {
		if (w->hover_control == this)
			w->hover_control = nullptr;
		if (w->focus_control == this)
			w->focus_control = nullptr;
	}
	for (auto cc: get_children(ChildFilter::All))
		cc->_unregister_from_window();
}

void Control::_unregister() {
	if (!owner)
		return;
	_unregister_from_window();
	if (type == ControlType::Panel)
		return;
	for (auto cc: get_children(ChildFilter::All))
		cc->_unregister();
	// NOPE we might replace controls but want to keep the event handlers! (e.g. for common toolbar buttons)
	/*base::remove_if(owner->event_handlers, [this] (const Panel::EventHandler& e) {
		return e.id == id;
	});*/
	base::remove(owner->controls, this);
	owner = nullptr;
}

Array<Control*> Control::get_children(ChildFilter f) {
	Array<Control*> r;
	const auto cc = _get_children(f);
	r.simple_assign(&cc);
	return r;
}


void Control::request_redraw() {
	if (auto w = get_window())
		w->redraw(id);
	//else
	//msg_write(id + "  can not refresh  " + p2s(owner));
}

bool Control::has_focus() const {
	if (auto w = get_window())
		return w->focus_control == this;
	return false;
}

void Control::prevent_event_propagation() {
	if (auto w = get_window())
		w->allow_event_propagation = false;
}

void Control::enable(bool _enabled) {
	enabled = _enabled;
	request_redraw();
}


Window* Control::get_window() const {
	if (auto w = as_window(const_cast<Control*>(this)))
		return w;
	if (owner)
		return owner->get_window();
	return nullptr;
}

void Control::on_mouse_move(const vec2& m, const vec2& d) {
	emit_event(event_id::MouseMove, false);
}

bool Control::emit_event(const string& msg, bool is_default) {
	if (owner)
		return owner->handle_event(id, msg, is_default);
	return false;
}

void Control::set_option(const string& key, const string& value) {
	if (key == "ignorehover") {
		ignore_hover = true;
	} else if (key == "ignorefocus") {
		can_grab_focus = false;
	} else if (key == "cangrabfocus") {
		can_grab_focus = value._bool() or (value == "");
	} else if (key == "grabfocus") {
		can_grab_focus = true;
		run_later(0.01f, [this] {
			// wait till we have a window
			if (auto w = get_window())
				w->focus_control = this;
		});
	} else if (key == "disabled") {
		enable(false);
	} else if (key == "enabled") {
		enable(value._bool() or value == "");
	} else if (key == "tooltip") {
		tooltip = value;
	} else {
		Node::set_option(key, value);
		request_redraw();
	}
}


}
