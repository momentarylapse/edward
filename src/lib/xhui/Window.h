#pragma once

#include "../base/base.h"
#include "../math/vec2.h"
#include <GLFW/glfw3.h>
#include <functional>
#include "Panel.h"

namespace xhui {

class Control;
class HeaderBar;
class Painter;
class Dialog;
class Context;

enum class Flags;

struct Event {
	enum class Type {
		MouseMove,
		Scroll,
		ButtonDown,
		ButtonUp,
		KeyDown,
		KeyUp
	} type;
	vec2 param1 = {0, 0};
	int param2 = 0;
	int param3 = 0;
};

class Window : public Panel {
	friend class Painter;
	friend class Control;
	friend class Panel;
	friend class Dialog;
	friend class Context;
public:

	Window(const string& title, int width, int height);
	Window(const string& title, int width, int height, Flags flags);
	~Window() override;

	void _handle_events();
	void _compress_events();

	void redraw(const string& id);

	string get_title() const { return title; }
	void set_title(const string& t);

	void get_position(int& x, int& y);
	void set_position(int x, int y);

	void maximize(bool maximized);
	bool is_maximized() const;

	void request_destroy();

	bool button(int index) const;
	bool button_down(int index) const;
	bool is_key_pressed(int key) const;
	void set_mouse_mode(int mode);
	vec2 mouse_position() const;

	struct InputState {
		vec2 m;
		vec2 scroll;
		bool lbut, mbut, rbut;
		bool key[256];
		int key_code;
		int key_char;
	} state, state_prev;

	void focus(const string& id);

	float ui_scale;

protected:
	GLFWwindow *window;

	Array<Event> event_stack;

	static void _key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void _char_callback(GLFWwindow* window, unsigned int codepoint);
	static void _cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
	static void _cursor_enter_callback(GLFWwindow *window, int enter);
	static void _mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
	static void _scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
	static void _refresh_callback(GLFWwindow *window);
	static void _resize_callback(GLFWwindow* window, int width, int height);


	void _on_left_button_down(const vec2& m);
	void _on_left_button_up(const vec2& m);
	void _on_left_double_click(const vec2& m);
	void _on_middle_button_down(const vec2& m);
	void _on_middle_button_up(const vec2& m);
	void _on_right_button_down(const vec2& m);
	void _on_right_button_up(const vec2& m);
	void _on_mouse_move(const vec2 &m, const vec2& d);
	void _on_mouse_enter(const vec2& m);
	void _on_mouse_leave(const vec2& m);
	void _on_mouse_wheel(const vec2 &d);
	void _on_key_down(int key);
	void _on_key_up(int key);
	void _on_key_char(int character);

	bool _refresh_requested = true;
	void _on_draw();

	Control* get_hover_control(const vec2& p);

	Control* hover_control = nullptr;
	Control* focus_control = nullptr;
	void _clear_hover();
	Flags flags;
	string title;

	HeaderBar* header_bar = nullptr;

	Context* context = nullptr;
	bool first_draw = true;

	struct EventKeyCode {
		string id;
		int key_code;
	};
	Array<EventKeyCode> event_key_codes;

public:
	struct Drag {
		bool active = false;
		Control* source = nullptr;
		float pre_distance = -1;
		string title;
		string payload;
		vec2 m;
	} drag;
	void start_pre_drag(Control* source);
	void start_drag(const string& title, const string& payload);

	bool _destroy_requested = false;

	void set_key_code(const string& id, int key_code);

	shared_array<Dialog> dialogs;
};

class WindowX : public Window {
public:
	WindowX(const string &title, int width, int height);
};

Window* as_window(Control* c);

}
