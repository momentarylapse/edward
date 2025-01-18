#pragma once

#include "controls/Control.h"
#include <functional>

class Painter;

namespace xhui {

using Callback = std::function<void()>;
using CallbackP = std::function<void(::Painter*)>;

class Panel : public Control {
public:
	explicit Panel(const string &id);

	void _draw(Painter *p) override;
	void negotiate_area(const rect &available) override;

	void add(Control *c);


	void event(const string &id, Callback f);
	void event_x(const string &id, const string &msg, Callback f);
	void event_xp(const string &id, const string &msg, CallbackP f);

	Window *window = nullptr;
	Control *top_control = nullptr;
	Array<Control*> controls;

	class EventHandler {
	public:
		EventHandler() {};
		string id, msg;
		Callback f;
		CallbackP fp;
	};
	Array<EventHandler> event_handlers;

	void handle_event(const string &id, const string &msg, bool is_default);
	void handle_event_p(const string &id, const string &msg, Painter *p);

};

}