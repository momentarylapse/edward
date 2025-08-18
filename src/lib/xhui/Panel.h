#pragma once

#include "controls/Control.h"
#include <functional>
#include <lib/base/future.h>

class Painter;

namespace xhui {

using Callback = std::function<void()>;
using CallbackP = std::function<void(::Painter*)>;
struct Resource;
class Dialog;

class Panel : public Control {
public:
	explicit Panel(const string &id);

	void _draw(Painter* p) override;
	void negotiate_area(const rect& available) override;
	vec2 get_content_min_size() const override;
	vec2 get_greed_factor() const override;
	Array<Control*> get_children(ChildFilter f) const override;

	void add_child(shared<Control> c, int x=0, int y=0) override;
	void set_target(const string& id);

	void set_string(const string& id, const string& text);
	void add_string(const string& id, const string& text);
	void set_float(const string& id, float value);
	void set_int(const string& id, int value);
	void set_color(const string& id, const color& c);
	void check(const string& id, bool checked);
	void enable(const string& id, bool enabled);
	void expand(const string& id, bool expanded);
	void reset(const string& id);
	void set_visible(const string& id, bool visible);
	void activate(const string& id);
	string get_string(const string& id) const;
	float get_float(const string& id) const;
	int get_int(const string& id) const;
	bool is_checked(const string& id) const;
	color get_color(const string& c) const;
	void set_options(const string& id, const string& options);
	void set_option(const string& key, const string& value) override;


	int event(const string& id, Callback f);
	int event_x(const string& id, const string& msg, Callback f);
	int event_xp(const string& id, const string& msg, CallbackP f);
	void remove_event_handler(int uid);

	shared<Control> top_control;
	Control* target_control = nullptr;
	Array<Control*> controls;
	float padding;

	struct EventHandler {
		EventHandler() = default;
		string id, msg;
		Callback f;
		CallbackP fp;
		int uid;
	};
	Array<EventHandler> event_handlers;

	bool handle_event(const string& id, const string& msg, bool is_default);
	bool handle_event_p(const string& id, const string& msg, Painter* p);


	void add_control(const string& type, const string& title, int x, int y, const string& id);
	void _add_control(const string& ns, const Resource& cmd, const string& parent_id);
	void remove_control(const string& id);
	void remove_control(Control* c);
	Control* get_control(const string& id);
	void embed(const string& target, int x, int y, shared<Panel> p);
	void unembed(Panel* p);

	base::future<void> open_dialog(shared<Dialog> dialog);
	void close_dialog(Dialog* dialog);

	void from_source(const string& source);
	void from_resource(const Resource& resource);
	void from_resource(const string& id);

	template<class F>
	void for_control(const string& id, F f) {
		auto ccc = controls; // some controls might register new children here...
		for (auto& c: ccc)
			if (c->id == id)
				f(c);
		if (this->id == id)
			f(this);
	}
};

Panel* as_panel(Control* c);

}