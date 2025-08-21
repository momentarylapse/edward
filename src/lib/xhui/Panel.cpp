#include "Panel.h"
#include "Dialog.h"
#include "Painter.h"
#include "language.h"
#include "Resource.h"
#include "Theme.h"
#include "controls/Button.h"
#include "controls/CheckBox.h"
#include "controls/ColorButton.h"
#include "controls/ComboBox.h"
#include "controls/DrawingArea.h"
#include "controls/Edit.h"
#include "controls/Expander.h"
#include "controls/FileSelector.h"
#include "controls/Grid.h"
#include "controls/Group.h"
#include "controls/Image.h"
#include "controls/Label.h"
#include "controls/ListView.h"
#include "controls/MenuBar.h"
#include "controls/MultilineEdit.h"
#include "controls/Overlay.h"
#include "controls/RadioButton.h"
#include "controls/Separator.h"
#include "controls/Slider.h"
#include "controls/SpinButton.h"
#include "controls/TabControl.h"
#include "controls/ToggleButton.h"
#include "controls/Toolbar.h"
#include "controls/Viewport.h"
#include "../base/algo.h"
#include "../os/msg.h"

namespace xhui {



Panel::Panel(const string &_id) : Control(_id, ControlType::Panel) {
	ignore_hover = true;

	padding = 0;
	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Panel::_draw(Painter *p) {
	if (top_control and top_control->visible)
		top_control->_draw(p);
}

void Panel::negotiate_area(const rect &available) {
	_area = available;
	if (top_control)
		top_control->negotiate_area(_area.grow(-padding));

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

vec2 Panel::get_content_min_size() const {
	if (top_control)
		return top_control->get_effective_min_size() + vec2(padding, padding) * 2;
		//return top_control->get_content_min_size();
	return {0, 0};
}

vec2 Panel::get_greed_factor() const {
	vec2 f = {0, 0};
	if (top_control)
		f = top_control->get_greed_factor();
	if (size_mode_x == SizeMode::Expand)
		f.x = 1;
	else if (size_mode_x == SizeMode::Shrink)
		f.x = 0;
	if (size_mode_y == SizeMode::Expand)
		f.y = 1;
	else if (size_mode_y == SizeMode::Shrink)
		f.y = 0;
	return f;
}



void Panel::add_child(shared<Control> c, int x, int y) {
	if (target_control) {
		target_control->add_child(c, x, y);
	} else {
		top_control = c;
	}
	// don't register sub-panels!
	if (c->type != ControlType::Panel)
		c->_register(this);
	request_redraw();
}

void Panel::set_target(const string& id) {
	for (auto& c: controls)
		if (c->id == id)
			target_control = c;
}

static int event_next_uid = 0;

int Panel::event(const string &id, Callback f) {
	EventHandler e;
	e.uid = event_next_uid ++;
	e.id = id;
	e.f = f;
	event_handlers.add(e);
	return e.uid;
}

int Panel::event_x(const string &id, const string &msg, Callback f) {
	EventHandler e;
	e.uid = event_next_uid ++;
	e.id = id;
	e.msg = msg;
	e.f = f;
	event_handlers.add(e);
	return e.uid;
}

int Panel::event_xp(const string &id, const string &msg, CallbackP f) {
	EventHandler e;
	e.uid = event_next_uid ++;
	e.id = id;
	e.msg = msg;
	e.fp = f;
	event_handlers.add(e);
	return e.uid;
}

void Panel::remove_event_handler(int uid) {
	base::remove_if(event_handlers, [uid] (const EventHandler& e) {
		return e.uid == uid;
	});
}

bool match_event_id(Panel::EventHandler& e, const string &id) {
	if (e.id == id)
		return true;
	if (e.id.find("*") >= 0)
		return id.match(e.id);
	return false;
}

bool match_event(Panel::EventHandler& e, const string &id, const string &msg, bool is_default) {
	if (!match_event_id(e, id))
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

void Panel::add_string(const string& id, const string& text) {
	auto ccc = controls; // some controls might register new children here...
	for (auto& c: ccc)
		if (c->id == id)
			c->add_string(text);
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

void Panel::check(const string& id, bool value) {
	for (auto& c: controls)
		if (c->id == id)
			c->check(value);
}

void Panel::set_color(const string& id, const color& col) {
	for (auto& c: controls)
		if (c->id == id)
			c->set_color(col);
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

bool Panel::is_checked(const string& id) const {
	for (auto& c: controls)
		if (c->id == id)
			return c->is_checked();
	return false;
}

color Panel::get_color(const string& id) const {
	for (auto& c: controls)
		if (c->id == id)
			return c->get_color();
	return Black;
}

void Panel::reset(const string& id) {
	for (auto& c: controls)
		if (c->id == id) {
			c->reset();
			return; // we might delete controls here...
		}
}

void Panel::enable(const string& id, bool enabled) {
	for (auto& c: controls)
		if (c->id == id)
			c->enable(enabled);
}


void Panel::expand(const string& id, bool expanded) {
	for (auto& c: controls)
		if (c->id == id)
			c->expand(expanded);
}

void Panel::set_visible(const string& id, bool visible) {
	for (auto& c: controls)
		if (c->id == id)
			c->visible = visible;
	request_redraw();
}

void Panel::activate(const string &id) {
	for (auto& c: controls)
		if (c->id == id and c->can_grab_focus)
			if (auto w = get_window()) {
				w->focus_control = c;
				request_redraw();
			}
}


void Panel::set_options(const string& id, const string& options) {
	for_control(id, [&options] (Control* c) {
		for (const auto& o: options.explode(",")) {
			auto xx = o.explode("=");
			if (xx.num >= 2)
				c->set_option(xx[0], xx[1]);
			else
				c->set_option(xx[0], "");
		}
	});
}

void Panel::set_option(const string& key, const string& value) {
	if (key == "padding") {
		padding = value._float();
	} else {
		Control::set_option(key, value);
	}
}



Array<Control*> Panel::get_children(ChildFilter) const {
	if (top_control)
		return {top_control.get()};
	return {};
}


void Panel::add_control(const string &type, const string &_title, int x, int y, const string &id) {
	//printf("HuiPanelAddControl %s  %s  %d  %d  %s\n", type.c_str(), _title.c_str(), x, y, id.c_str());
	string title = _title;
	if (title.head(1) == "!") {
		int p0 = title.find("\\");
		if (p0 > 0)
			title = title.sub(p0 + 1);
	}
	if (type == "Button")
		add_child(new Button(id, title), x, y);
	else if (type == "CheckBox")
		add_child(new CheckBox(id, title), x, y);
	else if (type == "ColorButton")
		add_child(new ColorButton(id), x, y);
	else if (type == "ComboBox")
		add_child(new ComboBox(id, title), x, y);
	else if (type == "DrawingArea")
		add_child(new DrawingArea(id), x, y);
	else if (type == "Edit")
		add_child(new Edit(id, title), x, y);
	else if (type == "Expander")
		add_child(new Expander(id, title), x, y);
	else if (type == "FileSelector")
		add_child(new FileSelector(id), x, y);
	else if (type == "Grid")
		add_child(new Grid(id), x, y);
	else if (type == "Group")
		add_child(new Group(id, title), x, y);
	else if (type == "Image")
		add_child(new Image(id, title), x, y);
	else if (type == "Label")
		add_child(new Label(id, title), x, y);
	else if (type == "ListView")
		add_child(new ListView(id, title), x, y);
	else if (type == "MenuBar")
		add_child(new MenuBar(id), x, y);
	else if (type == "MultilineEdit")
		add_child(new MultilineEdit(id, title), x, y);
	else if (type == "Overlay")
		add_child(new Overlay(id), x, y);
	else if (type == "RadioButton")
		add_child(new RadioButton(id, title), x, y);
	else if (type == "Separator")
		add_child(new Separator(id, Orientation::VERTICAL), x, y);
	else if (type == "Slider")
		add_child(new Slider(id), x, y);
	else if (type == "SpinButton")
		add_child(new SpinButton(id, title._float()), x, y);
	else if (type == "TabControl")
		add_child(new TabControl(id, title), x, y);
	else if (type == "ToggleButton")
		add_child(new ToggleButton(id, title), x, y);
	else if (type == "Toolbar")
		add_child(new Toolbar(id), x, y);
	else if (type == "Viewport")
		add_child(new Viewport(id), x, y);
//	else if (type == "TreeView")
//		add_tree_view(title, x, y, id);
//	else if (type == "ProgressBar")
//		add_progress_bar(title, x, y, id);
/*	else if ((type == "Expander") or (type == "Revealer"))
		add_expander(title, x, y, id);
	else if (type == "Scroller")
		add_scroller(title, x, y, id);
	else if (type == "Paned")
		add_paned(title, x, y, id);
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

void Panel::remove_control(Control* ccc) {
	// no need to be efficient :/
	for (auto c: controls)
		for (auto cc: c->get_children(ChildFilter::All))
			if (cc == ccc) {
				c->remove_child(cc);
				request_redraw();
				return;
			}
}

void Panel::remove_control(const string& id) {
	if (auto c = get_control(id))
		remove_control(c);
}

Control* Panel::get_control(const string& id) {
	for (auto c: controls)
		if (c->id == id)
			return c;
	return nullptr;
}



void Panel::embed(const string& target, int x, int y, shared<Panel> p) {
	set_target(target);
	add_child(p.to<Control>(), x, y);
	p->owner = this;
	request_redraw();
}

void Panel::unembed(Panel* p) {
	remove_control(p);
	p->owner = nullptr;
}



void Panel::from_source(const string& source) {
	from_resource(parse_resource(source));
}
void Panel::from_resource(const Resource& res) {
	bool res_is_window = ((res.type == "Dialog") or (res.type == "Window"));

	//	set_id(res.id);
	id = res.id;

	if (res_is_window) {
		int width = res.value("width", "0")._int();
		int height = res.value("height", "0")._int();
		for (auto &o: res.options)
			set_options(id, o);

		if (auto dlg = as_dialog(this)) {
			if (width + height > 0) {
				dlg->width = width;
				dlg->height = height;
			}
			dlg->set_title(get_language(res.id, res.id));
		} else if (auto win = as_window(this)) {
			/*if (width + height > 0) {
				win->width = width;
				win->height = height;
			}*/
			win->title = get_language(res.id, res.id);

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
	}

#if 0
	// directly change window?
	if (panel_is_window and res_is_window) {

		// title
		//window->set_title(get_language(res.id, res.id));

		// size
		int width = res.value("width", "0")._int();
		int height = res.value("height", "0")._int();
			msg_write("DDDDD");
			if (width + height > 0) {
				dlg->width = width;
				dlg->height = height;
			}
			dlg->title = get_language(res.id, res.id);
		}
//			window->set_size(width, height);

	}
#endif

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

void Panel::from_resource(const string& id) {
	if (auto cmd = get_resource(id))
		from_resource(*cmd);
	else
		msg_error("resource id not found: " + id);
}

base::future<void> Panel::open_dialog(shared<Dialog> dialog) {
	dialog->owner = this;
	if (auto w = get_window()) {
		w->dialogs.add(dialog.get());
	}
	request_redraw();
	return dialog->basic_promise.get_future();
}

void Panel::close_dialog(Dialog* dialog) {
	shared<Dialog> keep_alive = dialog;

	for (int i=0; i<controls.num; i++)
		if (controls[i] == (Control*)dialog) {
			controls.erase(i);
		}
	if (auto w = get_window()) {
		w->dialogs.pop();
		w->hover_control = nullptr;
		w->focus_control = nullptr;
	}
	dialog->basic_promise();
	request_redraw();
}



Panel* as_panel(Control* c) {
	if (c->type == ControlType::Panel or c->type == ControlType::Window)
		return static_cast<Panel*>(c);
	return nullptr;
}


}
