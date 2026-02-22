#include "Window.h"
#include "xhui.h"
#include "Painter.h"
#include "Context.h"
#include "Dialog.h"
#include "Theme.h"
#include "controls/Control.h"
#include "controls/HeaderBar.h"
#include "../os/time.h"
#include "../os/msg.h"
#include <lib/base/algo.h>
#include <lib/base/optional.h>


namespace xhui {

Array<Window*> _windows_;

Window::Window(const string &title, int w, int h) : Window(title, w, h, Flags::NONE) {}

Window::Window(const string &_title, int w, int h, Flags _flags) : Panel(":window:") {
	type = ControlType::Window;
	title = _title;
	flags = _flags;
	ui_scale = global_ui_scale;
	window = nullptr;
	memset(&state, 0, sizeof(state));
	memset(&state_prev, 0, sizeof(state_prev));

	if (flags & Flags::OWN_DECORATION) {
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	} else {
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
	}

	if (!(flags & Flags::FAKE)) {
		window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);

		if (flags & Flags::OWN_DECORATION) {
			if (glfwGetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER))
				msg_write("TRANSPARENT");
			else
				msg_write("NOT TRANSPARENT");
			header_bar = new HeaderBar(this, ":headerbar:");
		}

		glfwGetWindowContentScale(window, &ui_scale, nullptr);

		glfwSetWindowUserPointer(window, this);

		padding = Theme::_default.window_margin;

		glfwSetKeyCallback(window, _key_callback);
		glfwSetCharCallback(window, _char_callback);
		glfwSetCursorPosCallback(window, _cursor_position_callback);
		glfwSetCursorEnterCallback(window, _cursor_enter_callback);
		glfwSetMouseButtonCallback(window, _mouse_button_callback);
		glfwSetScrollCallback(window, _scroll_callback);
		glfwSetWindowRefreshCallback(window, _refresh_callback);
		glfwSetWindowSizeCallback(window, _resize_callback);

		_windows_.add(this);
	}
}

Window::~Window() {
	if (window)
		glfwDestroyWindow(window);

	for (int i=0; i<_windows_.num; i++)
		if (_windows_[i] == this)
			_windows_.erase(i);
}


/*bool Window::get_key_up(int k) {
	if (k < 0 or k >= 256)
		return false;
	return !state.key[k] and state_prev.key[k];
}*/

int key_decode(int key) {
	for (int i=0; i<10; i++)
		if (key == GLFW_KEY_0 + i)
			return xhui::KEY_0 + i;
	for (int i=0; i<26; i++)
		if (key == GLFW_KEY_A + i)
			return xhui::KEY_A + i;
	for (int i=0; i<12; i++)
		if (key == GLFW_KEY_F1 + i)
			return xhui::KEY_F1 + i;
	if (key == GLFW_KEY_MINUS)
		return xhui::KEY_MINUS;
	if (key == GLFW_KEY_PERIOD)
		return xhui::KEY_DOT;
	if (key == GLFW_KEY_COMMA)
		return xhui::KEY_COMMA;

	if (key == GLFW_KEY_ENTER)
		return xhui::KEY_RETURN;
	if (key == GLFW_KEY_SPACE)
		return xhui::KEY_SPACE;
	if (key == GLFW_KEY_BACKSPACE)
		return xhui::KEY_BACKSPACE;
	if (key == GLFW_KEY_ESCAPE)
		return xhui::KEY_ESCAPE;
	if (key == GLFW_KEY_UP)
		return xhui::KEY_UP;
	if (key == GLFW_KEY_DOWN)
		return xhui::KEY_DOWN;
	if (key == GLFW_KEY_LEFT)
		return xhui::KEY_LEFT;
	if (key == GLFW_KEY_RIGHT)
		return xhui::KEY_RIGHT;
	if (key == GLFW_KEY_LEFT_SHIFT)
		return xhui::KEY_LSHIFT;
	if (key == GLFW_KEY_RIGHT_SHIFT)
		return xhui::KEY_RSHIFT;
	if (key == GLFW_KEY_LEFT_CONTROL)
		return xhui::KEY_LCONTROL;
	if (key == GLFW_KEY_RIGHT_CONTROL)
		return xhui::KEY_RCONTROL;
	if (key == GLFW_KEY_LEFT_ALT)
		return xhui::KEY_LALT;
	if (key == GLFW_KEY_RIGHT_ALT)
		return xhui::KEY_RALT;
	/*if (key == GLFW_KEY_LEFT_SUPER)
		return xhui::KEY_LSUPER;
	if (key == GLFW_KEY_RIGHT_SUPER)
		return xhui::KEY_RSUPER;*/
	if (key == GLFW_KEY_PAGE_UP)
		return xhui::KEY_PAGE_UP;
	if (key == GLFW_KEY_PAGE_DOWN)
		return xhui::KEY_PAGE_DOWN;
	if (key == GLFW_KEY_HOME)
		return xhui::KEY_HOME;
	if (key == GLFW_KEY_END)
		return xhui::KEY_END;
	if (key == GLFW_KEY_DELETE)
		return xhui::KEY_DELETE;
	if (key == GLFW_KEY_INSERT)
		return xhui::KEY_INSERT;
	if (key == GLFW_KEY_TAB)
		return xhui::KEY_TAB;
	return -1;
}

int mods_decode(int mods) {
	int r = 0;
	if (mods & GLFW_MOD_SHIFT)
		r += KEY_SHIFT;
	if (mods & GLFW_MOD_CONTROL)
		r += KEY_CONTROL;
	if (mods & GLFW_MOD_ALT)
		r += KEY_ALT;
	if (mods & GLFW_MOD_SUPER)
		r += KEY_SUPER;
	return r;
}


void Window::_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	//msg_write(format("  glfw:  %d", key));
	int k = key_decode(key);
	if (k < 0)
		return;

	auto w = (Window*)glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS or action == GLFW_REPEAT) {
		w->state.key[k] = true;
	} if (action == GLFW_RELEASE) {
		w->state.key[k] = false;
	}

	k += mods_decode(mods);
	//msg_write(format("key  %d  %d  %d  %d", k, key, action, mods));

	w->state.key_code = k;
	w->state.key_char = 0;

	if (action == GLFW_PRESS or action == GLFW_REPEAT) {
		//w->state.key
		w->_on_key_down(k);
	} if (action == GLFW_RELEASE) {
		w->_on_key_up(k);
	}
}

void Window::_char_callback(GLFWwindow* window, unsigned int codepoint) {
	//msg_write(format("  glfw  char:  %d", codepoint));
	auto w = (Window*)glfwGetWindowUserPointer(window);
	w->state.key_char = (int)codepoint;
	w->_on_key_char((int)codepoint);
}


static bool resync_next_mouse_move = false;

void Window::_cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
	//msg_write(format("mouse %f  %f", xpos, ypos));
	auto w = (Window*)glfwGetWindowUserPointer(window);
	Event e;
	e.type = Event::Type::MouseMove;
//#ifdef OS_MAC
	e.param1 = {(float)xpos, (float)ypos};
/*#else
	// why?!? this should be consistent...
	e.param1 = {(float)xpos / w->ui_scale, (float)ypos / w->ui_scale};
#endif*/
	w->event_stack.add(e);
}

void Window::_cursor_enter_callback(GLFWwindow *window, int enter) {
	auto w = (Window*)glfwGetWindowUserPointer(window);
	if (enter == 1)
		w->_on_mouse_enter(w->state.m);
	else
		w->_on_mouse_leave(w->state.m);
}

void Window::_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	//std::cout << "button " << button << " " << action << " " << mods << "\n";
	auto w = (Window*)glfwGetWindowUserPointer(window);
	static os::Timer timer;
	if (action == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			w->_on_left_button_down(w->state.m);
			float dt = timer.get();
			if (dt > 0.05f and dt < 0.5f)
				w->_on_left_double_click(w->state.m);
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			w->_on_middle_button_down(w->state.m);
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			w->_on_right_button_down(w->state.m);
	} else if (action == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			w->_on_left_button_up(w->state.m);
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			w->_on_middle_button_up(w->state.m);
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			w->_on_right_button_up(w->state.m);
	}
}

void Window::_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	//std::cout << "scroll " << xoffset << " " << yoffset << "\n";
	auto w = (Window*)glfwGetWindowUserPointer(window);

	Event e;
	e.type = Event::Type::Scroll;
	e.param1 = {(float)xoffset, (float)yoffset};
	w->event_stack.add(e);

	/*w->state.scroll.x = xoffset;
	w->state.scroll.y = yoffset;
	w->_on_mouse_wheel(w->state.scroll);*/
}

void Window::_refresh_callback(GLFWwindow *window) {
	auto w = (Window*)glfwGetWindowUserPointer(window);
	w->_refresh_requested = true;
}

void Window::_resize_callback(GLFWwindow* window, int width, int height) {
	//msg_write(format("resize %d  %d", width, height));
	auto w = (Window*)glfwGetWindowUserPointer(window);

	if (w->context)
		w->context->resize(width, height);
	w->_refresh_requested = true;
}

void Window::redraw(const string &id) {
	_refresh_requested = true;
}

void Window::get_position(int &x, int &y) {
	glfwGetWindowPos(window, &x, &y);
}

void Window::set_position(int x, int y) {
	glfwSetWindowPos(window, x, y);
}

void Window::maximize(bool maximized) {
	if (maximized)
		glfwMaximizeWindow(window);
	else
		glfwRestoreWindow(window);
}

bool Window::is_maximized() const {
	return glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
}



// TODO: widget offset?
void Window::_on_left_button_down(const vec2& m) {
	state.lbut = true;
	if (hover_control) {
		if (hover_control->can_grab_focus) {
			focus_control = hover_control;
			redraw("");
		}
		hover_control->on_left_button_down(m);
	}
	on_left_button_down(m);
}
void Window::_on_left_button_up(const vec2& m) {
	state.lbut = false;
	if (drag.active and hover_control)
		hover_control->emit_event(event_id::DragDrop, false);
	drag.active = false;
	drag.source = nullptr;
	request_redraw();

	if (hover_control)
		hover_control->on_left_button_up(m);
	on_left_button_up(m);
}
void Window::_on_left_double_click(const vec2& m) {
	state.lbut = true;
	if (hover_control)
		hover_control->on_left_double_click(m);
	on_left_double_click(m);
}
void Window::_on_middle_button_down(const vec2& m) {
	state.mbut = true;
	if (hover_control)
		hover_control->on_middle_button_down(m);
	on_middle_button_down(m);
}
void Window::_on_middle_button_up(const vec2& m) {
	state.mbut = false;
	if (hover_control)
		hover_control->on_middle_button_up(m);
	on_middle_button_up(m);
}
void Window::_on_right_button_down(const vec2& m) {
	state.rbut = true;
	if (hover_control)
		hover_control->on_right_button_down(m);
	on_right_button_down(m);
}
void Window::_on_right_button_up(const vec2& m) {
	state.rbut = false;
	if (hover_control)
		hover_control->on_right_button_up(m);
	on_right_button_up(m);
}
void Window::_on_mouse_move(const vec2 &m, const vec2& d) {
	auto hover = get_hover_control(m);
	if (state.lbut and drag.source) {
		if (drag.active) {
			drag.m = m;
			// update events...

		} else if (drag.pre_distance < 10) {
			drag.pre_distance += d.length();
			if (drag.pre_distance > 10) {
				drag.source->emit_event(event_id::DragStart, false);
			}
		}
	}

	if (hover != hover_control and !state.lbut or drag.active) {
		if (hover_control)
			hover_control->on_mouse_leave(m);
		hover_control = hover;
		if (hover_control)
			hover_control->on_mouse_enter(m);
	}
	if (hover_control)
		hover_control->on_mouse_move(m, d);
	on_mouse_move(m, d);
}
void Window::_on_mouse_enter(const vec2& m) {
	on_mouse_enter(m);
}
void Window::_on_mouse_leave(const vec2& m) {
	if (hover_control and !state.lbut) {
		hover_control->on_mouse_leave(m);
		hover_control = nullptr;
	}
	on_mouse_leave(m);
}
void Window::_on_mouse_wheel(const vec2 &d) {
	if (hover_control)
		hover_control->on_mouse_wheel(d);
	on_mouse_wheel(d);
}

void Window::_on_key_down(int k) {
	for (const auto& e: event_key_codes)
		if (k == e.key_code)
			handle_event(e.id, event_id::Activate, true);
	if (focus_control)
		focus_control->on_key_down(k);
	if (dialogs.num > 0)
		dialogs.back()->on_key_down(k);
	on_key_down(k);
}

void Window::_on_key_up(int k) {
	if (focus_control)
		focus_control->on_key_up(k);
	on_key_up(k);
}

void Window::_on_key_char(int character) {
	if (focus_control)
		focus_control->on_key_char(character);
}


void Window::_on_draw() {
	if (!context)
		context = Context::create(this);
	auto p = context->prepare_draw();
	if (!p)
		return;
	auto a = p->area();
	_area = p->area();

	if (first_draw)
		handle_event_p(id, event_id::Initialize, p);
	first_draw = false;

	handle_event_p(id, event_id::JustBeforeDraw, p);
	context->begin_draw(p);

	if (flags & Flags::OWN_DECORATION) {
		p->clear(color(0,0,0,0));
		float R = Theme::_default.window_radius;

		// shadow
		p->accumulate_alpha = true;
		float R_shadow = 12;
		p->set_color({0.3f, 0,0,0});
		p->softness = R_shadow;
		p->set_roundness(R + R_shadow);
		p->draw_rect(a);
		p->softness = 0;

		a = a.grow(-R_shadow);

		rect header = rect(a.x1, a.x2, a.y1, a.y1 + Theme::_default.headerbar_height);

		// window border
		p->set_roundness(R+1);
		p->set_color(Theme::_default.border);
		p->draw_rect(a.grow(1));

		// main background
		p->accumulate_alpha = false;
		p->set_roundness(R);
		p->set_color(Theme::_default.background);
		p->draw_rect(a);

		// header
		if (header_bar) {
			header_bar->negotiate_area(header);
			header_bar->_draw(p);
		}

		a.y1 += Theme::_default.headerbar_height;
	} else {
		p->clear(Theme::_default.background);
	}

	// contents
	Panel::negotiate_area(a);
	Panel::_draw(p);

	for (auto dlg: dialogs) {
		p->set_color(color(0.3f, 0, 0, 0));
		p->draw_rect(a);
		dlg->negotiate_area(dlg->suggest_area(a));
		dlg->_draw(p);
	}

	if (drag.active) {
		p->set_font_size(Theme::_default.font_size * 1.5f);
		p->set_color(Red);
		p->draw_str(state.m + vec2(20, 0), drag.title);
		p->set_font_size(Theme::_default.font_size);
	}

#if 0
	if (hover_control) {
		p->set_color(Red);
		p->set_fill(false);
		p->draw_rect(hover_control->_area);
		p->set_fill(true);
	}

	static int frame = 0;
	frame ++;
	p->set_color(Red);
	p->draw_str({20,20}, str(frame));
#endif

	if (false) {
		// hover debug
		p->set_color(color(0.2f, 1, 0,0));
		if (hover_control)
			p->draw_rect(hover_control->_area);
	}

	_refresh_requested = false;
	context->end_draw(p);
}

void Window::_compress_events() {
	if (event_stack.num <= 1)
		return;
	// find last MouseMove event
	base::optional<Event> last_mouse_move;
	Event total_scroll = {Event::Type::Scroll, {0,0}, 0, 0};
	//int n = 0;
	for (auto& e: event_stack) {
		if (e.type == Event::Type::MouseMove)
			last_mouse_move = e;
		if (e.type == Event::Type::Scroll)
			total_scroll.param1 += e.param1;
	}

	// remove all other MouseMove events
	/*base::remove_if(event_stack, [n] (const Event& e) {
		return e.type == Event::Type::MouseMove or e.type == Event::Type::Scroll;
	});*/
	event_stack.clear();

	if (last_mouse_move)
		event_stack.add(*last_mouse_move);
	if (total_scroll.param1 != vec2(0,0))
		event_stack.add(total_scroll);
}

void Window::_handle_events() {
	_compress_events();
	for (const auto& e: event_stack) {
		if (e.type == Event::Type::MouseMove) {
			state_prev.m = state.m;
			state.m = e.param1;
			if (resync_next_mouse_move) {
				state_prev.m = state.m;
				resync_next_mouse_move = false;
			}
			_on_mouse_move(state.m, state.m - state_prev.m);
		} else if (e.type == Event::Type::Scroll) {
			state.scroll = e.param1;
			_on_mouse_wheel(e.param1);
		}
	}
	event_stack.clear();

	if (_refresh_requested)
		_on_draw();

	if (glfwWindowShouldClose(window)) {
		if (!handle_event(id, event_id::Close, false))
			request_destroy();
		// either we will handle the event on our own, or we want to ignore...
		glfwSetWindowShouldClose(window, GLFW_FALSE);
	}
}

void Window::set_title(const string& t) {
	title = t;
	glfwSetWindowTitle(window, t.c_str());
	redraw("");
}

Control *Window::get_hover_control(const vec2 &p) {
	Array<Control*> seeds;
	if (dialogs.num > 0) {
		seeds.append(dialogs.back()->get_children(ChildFilter::All));
	} else {
		if (header_bar)
			seeds.add(header_bar);
		seeds.add(top_control.get());
	}
	int cur_seed = 0;

	// we might need multiple seeds, if we encounter Overlays!

	Control* best = nullptr;
	while (cur_seed < seeds.num) {
		auto c = seeds[cur_seed ++];
		while (c) {
			if (c->_area.inside(p) and !c->ignore_hover and c->visible)
				best = c;
			Control* next = nullptr;
			for (auto cc: c->get_children(ChildFilter::OnlyActive))
				if (cc->_area.inside(p) and cc->visible) {
					if (next)
						seeds.add(cc);
					else
						next = cc;
				}
			c = next;
		}
	}
	return best;
}

void Window::focus(const string& id) {
	if (auto c = get_control(id)) {
		if (c->can_grab_focus)
			focus_control = c;
	}
}

void Window::_clear_hover() {
	if (hover_control)
		hover_control->on_mouse_leave(state.m);
	hover_control = nullptr;
}



bool Window::button(int index) const {
	if (index == 2)
		return state.rbut;
	if (index == 1)
		return state.mbut;
	if (index == 0)
		return state.lbut;
	return false;
}

bool Window::button_down(int index) const {
	if (index == 2)
		return state.rbut and !state_prev.rbut;
	if (index == 1)
		return state.mbut and !state_prev.mbut;
	if (index == 0)
		return state.lbut and !state_prev.lbut;
	return false;
}

bool Window::is_key_pressed(int key) const {
	if (key == KEY_CONTROL)
		return state.key[KEY_LCONTROL] or state.key[KEY_RCONTROL];
	if (key == KEY_SHIFT)
		return state.key[KEY_LSHIFT] or state.key[KEY_RSHIFT];
	if (key >= 0 and key < 256)
		return state.key[key];
	return false;
}

void Window::set_mouse_mode(int mode) {
	if (mode == 0) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	state_prev.m = state.m;
	resync_next_mouse_move = true;
}

vec2 Window::mouse_position() const {
	return state.m;
}


void Window::start_pre_drag(Control* source) {
	drag.pre_distance = 0;
	drag.source = source;
	drag.active = false;
}

void Window::start_drag(const string& title, const string& payload) {
	drag.title = title;
	drag.payload = payload;
	drag.active = true;
	request_redraw();
}

void Window::set_key_code(const string &id, int key_code) {
	// make sure, each id has only 1 code
	//   (multiple ids may have the same code)
	for (auto &e: event_key_codes)
		if (e.id == id) {
			e.key_code = key_code;
			return;
		}
	event_key_codes.add({id, key_code});
}


void Window::request_destroy() {
	_destroy_requested = true;
}

WindowX::WindowX(const string &title, int w, int h) : Window(title, w, h, Flags::OWN_DECORATION) {
}

Window* as_window(Control* c) {
	if (c->type == ControlType::Window)
		return static_cast<Window*>(c);
	return nullptr;
}

}
