#include "Panel.h"
#include "Painter.h"
#include "language.h"
#include "Resource.h"
#include "Theme.h"
#include "controls/Button.h"
#include "controls/CheckBox.h"
#include "controls/DrawingArea.h"
#include "controls/Edit.h"
#include "controls/Grid.h"
#include "controls/Group.h"
#include "controls/Label.h"
#include "controls/ListView.h"
#include "controls/MultilineEdit.h"
#include "controls/Overlay.h"
#include "controls/SpinButton.h"
#include "../os/msg.h"

namespace xhui {



Panel::Panel(const string &_id) : Control(_id) {
	ignore_hover = true;

	padding = 0;
	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Panel::_draw(Painter *p) {
	if (top_control)
		top_control->_draw(p);
}

void Panel::negotiate_area(const rect &available) {
	_area = available;
	if (top_control)
		top_control->negotiate_area(smaller_rect(_area, padding));

	/*Array<int> w, h;
	get_grid_min_sizes(w, h);
	int total_min_w, total_min_h;
	get_content_min_size(total_min_w, total_min_h);
	float diff_x = max(available.width() - total_min_w, 0.0f);
	float diff_y = max(available.height() - total_min_h, 0.0f);

	Array<float> gx, gy;
	get_grid_greed_factors(gx, gy);
	float total_greed_x, total_greed_y;
	get_greed_factor(total_greed_x, total_greed_y);

	float greed_to_x = (total_greed_x > 0) ? diff_x / total_greed_x : 0;
	float greed_to_y = (total_greed_y > 0) ? diff_y / total_greed_y : 0;

	for (int i=0; i<w.num; i++)
		w[i] += greed_to_x * gx[i];
	for (int i=0; i<h.num; i++)
		h[i] += greed_to_y * gy[i];

	for (auto &c: children) {
		int x0 = _area.x1;
		int y0 = _area.y1;
		for (int i=0; i<c.x; i++)
			x0 += w[i] + spacing;
		for (int i=0; i<c.y; i++)
			y0 += h[i] + spacing;
		c.control->negotiate_area(rect(x0, x0 + w[c.x], y0, y0 + h[c.y]));
	}*/
}

Window* Panel::get_window() {
	if (auto w = dynamic_cast<Window*>(this))
		return w;
	if (owner)
		return owner->get_window();
	return nullptr;
}


void Panel::add(Control *c, int x, int y) {
	if (target_control) {
		target_control->add(c, x, y);
	} else {
		top_control = c;
	}
	// don't register sub-panels!
	if (dynamic_cast<Panel*>(c) == nullptr)
		c->_register(this);
	request_redraw();
}

void Panel::add(Control *c) {
	add(c, 0, 0);
}

void Panel::set_target(const string& id) {
	for (auto& c: controls)
		if (c->id == id)
			target_control = c;
}

void Panel::event(const string &id, Callback f) {
	EventHandler e;
	e.id = id;
	e.f = f;
	event_handlers.add(e);
}

void Panel::event_x(const string &id, const string &msg, Callback f) {
	EventHandler e;
	e.id = id;
	e.msg = msg;
	e.f = f;
	event_handlers.add(e);
}

void Panel::event_xp(const string &id, const string &msg, CallbackP f) {
	EventHandler e;
	e.id = id;
	e.msg = msg;
	e.fp = f;
	event_handlers.add(e);
}

bool match_event(Panel::EventHandler& e, const string &id, const string &msg, bool is_default) {
	if (e.id != id)
		return false;
	if (e.msg == "" and is_default)
		return true;
	return e.msg == msg;
}

bool Panel::handle_event(const string &id, const string &msg, bool is_default) {
	bool any_match = false;
	for (auto &e: event_handlers)
		if (match_event(e, id, msg, is_default) and e.f) {
			e.f();
			any_match = true;
		}
	return any_match;
}

bool Panel::handle_event_p(const string &id, const string &msg, Painter *p) {
	bool any_match = false;
	for (auto &e: event_handlers)
		if (match_event(e, id, msg, false) and e.fp) {
			e.fp(p);
			any_match = true;
		}
	return any_match;
}

void Panel::set_string(const string& id, const string& text) {
	for (auto& c: controls)
		if (c->id == id)
			c->set_string(text);
}

void Panel::set_float(const string& id, float value) {
	for (auto& c: controls)
		if (c->id == id)
			c->set_float(value);
}

void Panel::set_int(const string& id, int value) {
	for (auto& c: controls)
		if (c->id == id)
			c->set_int(value);
}

string Panel::get_string(const string& id) const {
	for (auto& c: controls)
		if (c->id == id)
			return c->get_string();
	return "";
}

float Panel::get_float(const string& id) const {
	for (auto& c: controls)
		if (c->id == id)
			return c->get_float();
	return 0;
}

int Panel::get_int(const string& id) const {
	for (auto& c: controls)
		if (c->id == id)
			return c->get_int();
	return 0;
}


void Panel::enable(const string& id, bool enabled) {
	for (auto& c: controls)
		if (c->id == id)
			c->enable(enabled);
}

void Panel::set_options(const string& id, const string& options) {
	for (auto& c: controls)
		if (c->id == id)
			for (const auto& o: options.explode(",")) {
				auto xx = o.explode("=");
				if (xx.num >= 2)
					c->set_option(xx[0], xx[1]);
				else
					c->set_option(xx[0], "");
			}
}


Array<Control*> Panel::get_children() const {
	if (top_control)
		return {top_control};
	return {};
}


void Panel::add_control(const string &type, const string &_title, int x, int y, const string &id) {
	//printf("HuiPanelAddControl %s  %s  %d  %d  %s\n", type.c_str(), title.c_str(), x, y, id.c_str());
	string title = _title;
	if (title.head(1) == "!") {
		auto x = title.explode("\\");
		if (x.num >= 2)
			title = x[1];
	}
	if (type == "Button")
		add(new Button(id, title), x, y);
/*	else if (type == "ColorButton")
		add_color_button(title, x, y, id);
	else if (type == "DefButton")
		add_def_button(title, x, y, id);*/
	else if ((type == "Label") or (type == "Text"))
		add(new Label(id, title), x, y);
	else if (type == "Edit")
		add(new Edit(id, title), x, y);
	else if (type == "MultilineEdit")
		add(new MultilineEdit(id, title), x, y);
	else if (type == "Group")
		add(new Group(id, title), x, y);
	else if (type == "CheckBox")
		add(new CheckBox(id, title), x, y);
//	else if (type == "ComboBox")
//		add_combo_box(title, x, y, id);
//	else if (type == "TabControl")
//		add_tab_control(title, x, y, id);
	else if (type == "ListView")
		add(new ListView(id, title), x, y);
//	else if (type == "TreeView")
//		add_tree_view(title, x, y, id);
//	else if (type == "IconView")
//		add_icon_view(title, x, y, id);
//	else if (type == "ProgressBar")
//		add_progress_bar(title, x, y, id);
//	else if (type == "Slider")
//		add_slider(title, x, y, id);
//	else if (type == "Image")
//		add_image(title, x, y, id);
	else if (type == "DrawingArea")
		add(new DrawingArea(id), x, y);
	else if (type == "Grid")
		add(new Grid(id), x, y);
	else if (type == "Overlay")
		add(new Overlay(id), x, y);
	else if (type == "SpinButton")
		add(new SpinButton(id, title._float()), x, y);
/*	else if (type == "RadioButton")
		add_radio_button(title, x, y, id);
	else if (type == "ToggleButton")
		add_toggle_button(title, x, y, id);
	else if ((type == "Expander") or (type == "Revealer"))
		add_expander(title, x, y, id);
	else if (type == "Scroller")
		add_scroller(title, x, y, id);
	else if (type == "Paned")
		add_paned(title, x, y, id);
	else if (type == "Separator")
		add_separator(title, x, y, id);
	else if (type == "MenuButton")
		add_menu_button(title, x, y, id);*/
	else
		msg_error("unknown hui control: " + type);
}

void Panel::_add_control(const string &ns, const Resource &cmd, const string &parent_id) {
	//msg_write(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
	set_target(parent_id);
	add_control(cmd.type, get_language_r(ns, cmd),
				cmd.x, cmd.y,
				cmd.id);

	for (const string &o: cmd.options)
		set_options(cmd.id, o);

	enable(cmd.id, cmd.enabled());
	/*if (cmd.has("hidden"))
		hide_control(cmd.id, true);

	if (cmd.image().num > 0)
		set_image(cmd.id, cmd.image());


	string tooltip = get_language_t(ns, cmd.id, cmd.tooltip);
	if (tooltip.num > 0)
		set_tooltip(cmd.id, tooltip);*/

	for (const Resource &c: cmd.children)
		_add_control(ns, c, cmd.id);
}

void Panel::embed(const string& target, int x, int y, Panel* p) {
	set_target(target);
	add(p, x, y);
	p->owner = this;
	request_redraw();
}


void Panel::from_source(const string& source) {
	from_resource(parse_resource(source));
}
void Panel::from_resource(const Resource& res) {

	bool res_is_window = ((res.type == "Dialog") or (res.type == "Window"));
	auto window = get_window();
	bool panel_is_window = window and !owner;

	// directly change window?
	if (panel_is_window and res_is_window) {
	//	for (auto &o: res.options)
	//		window->__set_options(o);

		// title
		window->set_title(get_language(res.id, res.id));

		// size
		int width = res.value("width", "0")._int();
		int height = res.value("height", "0")._int();
//		if (width + height > 0)
//			window->set_size(width, height);

		// menu/toolbar?
		string toolbar = res.value("toolbar");
		string menu = res.value("menu");
//		if (menu != "")
//			window->set_menu(create_resource_menu(menu, this));
//		if (toolbar != "")
//			window->get_toolbar(TOOLBAR_TOP)->set_by_id(toolbar);

/*		for (const auto &c: res.children)
			if (c.type == "HeaderBar") {
				window->_add_headerbar();
				for (auto &cc: c.children)
					_add_control(id, cc, ":header:");
			}*/
	}

//	set_id(res.id);

	int bw = res.value("borderwidth", "-1")._int();
	if (bw >= 0)
		padding = bw;


	// controls
	if (res_is_window) {
		if (res.children.num > 0)
			_add_control(id, res.children[0], "");
	} else {
//		embed_resource(res, "", 0, 0);
	}
}



}
