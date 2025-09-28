//
// Created by Michael Ankele on 2025-02-11.
//

#include "Toolbar.h"
#include "Button.h"
#include "ToggleButton.h"
#include "Separator.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../Resource.h"
#include "../language.h"
#include "../../os/msg.h"

namespace xhui {

Toolbar::Toolbar(const string& id) : Grid(id) {
	//size_mode_y = SizeMode::Shrink;
	spacing = 2;
}

void Toolbar::add_item(const string& id, const string& title, const string& image) {
	int n = get_children(ChildFilter::All).num;
	auto b = new Button(id, title);
	b->min_width_user = 50;
	b->min_height_user = 50;
	b->size_mode_x = SizeMode::Shrink;
	b->size_mode_y = SizeMode::Shrink;
	b->padding = {7, 7, 7, 7};
	//height=50 width=50 noexpandx ignorefocus
	b->set_option("flat", "");
	add_child(b, n, 0);
}


void Toolbar::add_item_checkable(const string& id, const string& title, const string& image) {
	int n = get_children(ChildFilter::All).num;
	auto b = new ToggleButton(id, title);
	b->min_width_user = 50;
	b->min_height_user = 50;
	b->size_mode_x = SizeMode::Shrink;
	b->size_mode_y = SizeMode::Shrink;
	b->padding = {7, 7, 7, 7};
	//height=50 width=50 noexpandx ignorefocus
	b->set_option("flat", "");
	add_child(b, n, 0);
}

void Toolbar::add_separator() {
	int n = get_children(ChildFilter::All).num;
	add_child(new Separator("", Orientation::VERTICAL), n, 0);
}


void Toolbar::reset() {
	auto items = get_children(ChildFilter::All);
	for (auto c: items)
		remove_child(c);
}


// create and apply a toolbar bar resource id
void Toolbar::set_by_id(const string &id) {
	Resource *res = get_resource(id);
	if (!res) {
		msg_error("Toolbar.SetByID  :~~(");
		return;
	}
	from_resource(res);
}

void Toolbar::from_resource(Resource *res) {
	reset();
	id = res->id;
	//Configure(res->b_param[0], res->b_param[1]);
	for (Resource &cmd: res->children) {
		string title = get_lang(id, cmd.id, cmd.title, false);
		string tooltip = get_language_t(id, cmd.id, cmd.tooltip);
		if (tooltip.num == 0)
			tooltip = title;

		if (cmd.type == "Item") {
			if (sa_contains(cmd.options, "checkable"))
				add_item_checkable(cmd.id, title, cmd.image());
			else
				add_item(cmd.id, title, cmd.image());
//			items.back()->set_tooltip(tooltip);
		} else if (cmd.type == "Separator") {
			add_separator();
		} else if (cmd.type == "Menu") {
			add_item(cmd.id, title, cmd.image());
			/*bool ok = false;
			for (string &o: cmd.options)
				if (o.find("menu=") == 0) {
					add_menu_by_id(title, cmd.image(), o.sub(5), cmd.id);
					items.back()->set_tooltip(get_language_t(id, cmd.id, cmd.tooltip));
					ok = true;
				}
			if (!ok and cmd.children.num > 0) {
				add_menu(title, cmd.image(), _create_res_menu_(id, &cmd, win), cmd.id);
				items.back()->set_tooltip(get_language_t(id, cmd.id, cmd.tooltip));
			}*/
		} else {
			continue;
		}

		Control* item = get_children(ChildFilter::All).back();
		for (auto &o: cmd.options) {
			auto xx = o.explode("=");
			if (xx.num >= 2)
				item->set_option(xx[0], xx[1]);
			else
				item->set_option(xx[0], "");
		}
	}
	enable(true);
}

void Toolbar::_draw(Painter* p) {
	if (is_main) {
		p->set_color(Theme::_default.background_raised());
		p->draw_rect(_area);
	}
	Grid::_draw(p);
}


void Toolbar::set_option(const string& key, const string& value) {
	if (key == "main") {
		is_main = true;
		margin.y2 = 3;
	} else if (key == "menu") {
		set_by_id(value);
	} else {
		Grid::set_option(key, value);
	}
}



} // xhui