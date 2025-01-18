#include "Window.h"
#include "xhui.h"
#include "Painter.h"
#include "ContextVulkan.h"
#include "Theme.h"
#include "controls/Control.h"
#include "controls/HeaderBar.h"
#include "../os/msg.h"


namespace xhui {

Array<Window*> _windows_;

Window::Window(const string &title, int w, int h) : Window(title, w, h, Flags::NONE) {}

Window::Window(const string &_title, int w, int h, Flags _flags) : Panel(":window:") {
	title = _title;
	flags = _flags;
	Panel::window = this;

	window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);

	if (flags & Flags::OWN_DECORATION) {
		glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		if (glfwGetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER))
			msg_write("TRANSPARENT");
		else
			msg_write("NOT TRANSPARENT");
		header_bar = new HeaderBar(this, ":headerbar:");
	}

	glfwSetWindowUserPointer(window, this);

	padding = Theme::_default.window_margin;

	glfwSetKeyCallback(window, _key_callback);
	glfwSetCursorPosCallback(window, _cursor_position_callback);
	glfwSetCursorEnterCallback(window, _cursor_enter_callback);
	glfwSetMouseButtonCallback(window, _mouse_button_callback);
	glfwSetScrollCallback(window, _scroll_callback);
	glfwSetWindowRefreshCallback(window, _refresh_callback);
	glfwSetWindowSizeCallback(window, _resize_callback);

	_windows_.add(this);
}

Window::~Window() {
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
	if (key == GLFW_KEY_ENTER)
		return xhui::KEY_RETURN;
	if (key == GLFW_KEY_SPACE)
		return xhui::KEY_SPACE;
	if (key == GLFW_KEY_BACKSPACE)
		return xhui::KEY_BACKSPACE;
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
	if (mods == GLFW_MOD_SHIFT)
		r += xhui::KEY_SHIFT;
	if (mods == GLFW_MOD_CONTROL)
		r += xhui::KEY_CONTROL;
	if (mods == GLFW_MOD_ALT)
		r += xhui::KEY_ALT;
	//if (mods == GLFW_MOD_SUPER)
	//	r += xhui::KEY_SUPER;
	return r;
}


void Window::_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	int k = key_decode(key);
	if (k < 0)
		return;

	auto w = (Window*)glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS) {
		w->state.key[k] = true;
	} if (action == GLFW_RELEASE) {
		w->state.key[k] = false;
	}

	k += mods_decode(mods);
	//std::cout << "key " << k << "    " << key << " " << action << " " << mods << "\n";

	if (action == GLFW_PRESS) {
		//w->state.key
		w->_on_key_down(k);
	} if (action == GLFW_RELEASE) {
		w->_on_key_up(k);
	}
}

void Window::_cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
	//msg_write(format("mouse %f  %f", xpos, ypos));
	auto w = (Window*)glfwGetWindowUserPointer(window);
	w->state.m.x = (float)xpos; // / w->ui_scale;
	w->state.m.y = (float)ypos; // / w->ui_scale;
	w->_on_mouse_move(w->state.m, {0,0});
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
	if (action == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			w->_on_left_button_down(w->state.m);
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
	w->state.scroll.x = xoffset;
	w->state.scroll.y = yoffset;
	w->_on_mouse_wheel(w->state.scroll);
}

void Window::_refresh_callback(GLFWwindow *window) {
	auto w = (Window*)glfwGetWindowUserPointer(window);
	w->_refresh_requested = true;
}

void Window::_resize_callback(GLFWwindow* window, int width, int height) {
	auto w = (Window*)glfwGetWindowUserPointer(window);
/*#if HAS_LIB_VULKAN
	if (w->context)
		w->context->resize(width, height);
#endif*/
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
	if (hover_control)
		hover_control->on_left_button_up(m);
	on_left_button_up(m);
}
void Window::_on_middle_button_down(const vec2& m) {
	on_middle_button_down(m);
}
void Window::_on_middle_button_up(const vec2& m) {
	on_middle_button_up(m);
}
void Window::_on_right_button_down(const vec2& m) {
	on_right_button_down(m);
}
void Window::_on_right_button_up(const vec2& m) {
	on_right_button_up(m);
}
void Window::_on_mouse_move(const vec2 &m, const vec2& d) {
	auto hover = get_hover_control(m);
	if (hover != hover_control and !state.lbut) {
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
	if (focus_control)
		focus_control->on_key_down(k);
	on_key_down(k);
}
void Window::_on_key_up(int k) {
	if (focus_control)
		focus_control->on_key_up(k);
	on_key_up(k);
}


void Window::_on_draw() {

#if HAS_LIB_VULKAN
	if (!context)
		context = new ContextVulkan(this);
#endif
	auto p = new Painter(this);
	auto a = p->area();

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

		a = smaller_rect(a, R_shadow);

		rect header = rect(a.x1, a.x2, a.y1, a.y1 + Theme::_default.headerbar_height);

		// window border
		p->set_roundness(R+1);
		p->set_color(Theme::_default.border);
		p->draw_rect(smaller_rect(a, -1));

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

#if 1
	if (top_control) {
		top_control->negotiate_area(smaller_rect(a, padding));
		top_control->_draw(p);
	}
#else
	p->set_color(Theme::_default.text);
	p->draw_rect({100, 200, 100, 200});
	p->set_color(Theme::_default.text);
	p->draw_str({300, 300}, "Test");
	p->draw_str({300, 500}, "Test 2");
#endif

	p->end();
	_refresh_requested = false;
	delete p;
}

void Window::_poll_events() {
	if (_refresh_requested)
		_on_draw();

	if (glfwWindowShouldClose(window)) {
//		msg_write("fake...close...");
//		exit(0);
		request_destroy();
	}
}

void Window::set_title(const string& t) {
	title = t;
	redraw("");
}

Control *Window::get_hover_control(const vec2 &p) {
	foreachb (auto c, controls)
		if (c->_area.inside(p) and !c->ignore_hover)
			return c;
	return nullptr;
}

void Window::request_destroy() {
	_destroy_requested = true;
}

WindowX::WindowX(const string &title, int w, int h) : Window(title, w, h, Flags::OWN_DECORATION) {
}

}
