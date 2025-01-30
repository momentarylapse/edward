#pragma once

#include "controls/Control.h"
#include <functional>

class Painter;

namespace xhui {

using Callback = std::function<void()>;
using CallbackP = std::function<void(::Painter*)>;
struct Resource;

class Panel : public Control {
public:
	explicit Panel(const string &id);

	void _draw(Painter* p) override;
	void negotiate_area(const rect& available) override;
	Array<Control*> get_children() const override;

	void add(Control* c);
	void add(Control* c, int x, int y) override;
	void set_target(const string& id);

	void set_string(const string& id, const string& text);
	void enable(const string& id, bool enabled);


	void event(const string& id, Callback f);
	void event_x(const string& id, const string& msg, Callback f);
	void event_xp(const string& id, const string& msg, CallbackP f);

	Window* window = nullptr;
	Control* top_control = nullptr;
	Control* target_control = nullptr;
	Array<Control*> controls;

	class EventHandler {
	public:
		EventHandler() {};
		string id, msg;
		Callback f;
		CallbackP fp;
	};
	Array<EventHandler> event_handlers;

	bool handle_event(const string &id, const string &msg, bool is_default);
	bool handle_event_p(const string &id, const string &msg, Painter *p);


	void add_control(const string &type, const string &title, int x, int y, const string &id);
	void _add_control(const string &ns, Resource &cmd, const string &parent_id);

};

}