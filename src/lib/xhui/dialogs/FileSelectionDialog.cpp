//
// Created by Michael Ankele on 2025-01-26.
//

#include "FileSelectionDialog.h"

#include <lib/base/iter.h>
#include <lib/os/msg.h>
#include <lib/xhui/Painter.h>
#include <lib/xhui/Theme.h>

#include "../controls/Button.h"
#include "../controls/Grid.h"
#include "../../os/filesystem.h"

namespace xhui {

class FileSelectionControl : public Control {
public:
	explicit FileSelectionControl(const string& id) : Control(id) {

	}
	void set_directory(const Path& dir) {
		hover = -1;
		selected = -1;
		current_dir = dir;
		if (dir.is_empty())
			current_dir = Application::initial_working_directory;
		items.clear();
		const auto list = os::fs::search(current_dir, filter, "df");
		for (const auto& e: list)
			if (str(e).head(1) != ".")
				items.add({e, os::fs::is_directory(current_dir | e)});
		request_redraw();
	}
	void _draw(Painter* p) override {
		p->set_color(Theme::_default.background_low);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(_area);
		p->set_color(Theme::_default.text);
		const auto clip0 = p->clip();
		p->set_clip(_area);
		for (const auto& [i, it]: enumerate(items)) {
			auto r = item_area(i);
			if (i == selected) {
				p->set_color(Theme::_default.background_low_selected);
				p->draw_rect(r);
			} else if (i == hover) {
				p->set_color(Theme::_default.background_hover);
				p->draw_rect(r);
			}
			p->set_color(it.is_directory ? color(1, 0.8f, 0.7f, 0) : Theme::_default.text_label);
			p->draw_rect({r.p00() + vec2(6, 6), r.p00() + vec2(20, 20)});
			p->set_color(Theme::_default.text_label);
			p->draw_str(r.p00() + vec2(26, 6), str(it.filename));
		}
		p->set_clip(clip0);
	}
	void on_mouse_move(const vec2& m, const vec2& d) override {
		hover = get_hover(m);
		request_redraw();
	}
	void on_mouse_leave(const vec2& m) override {
		hover = -1;
		request_redraw();
	}
	void on_left_button_down(const vec2& m) override {
		hover = get_hover(m);
		selected = hover;
		request_redraw();
		if (selected < 0) {
			emit_event(event_id::Select, false);
			return;
		}
		const auto e = items[selected];
		if (e.is_directory) {
			set_directory(current_dir | e.filename);
			emit_event("hui:change-directory", false);
		} else {
			emit_event(event_id::Select, false);
		}
	}
	int get_hover(const vec2& m) {
		for (const auto& [i, it]: enumerate(items))
			if (item_area(i).inside(m))
				return i;
		return -1;
	}
	rect item_area(int index) const {
		float dy = 30;
		float margin = 8;
		float space = 3;
		return rect(_area.p00() + vec2(margin, margin + index * dy), _area.p10() + vec2(-margin, margin + (index+1) * dy - space));
	}
	Path get_selected_filename() const {
		if (selected < 0)
			return "";
		return current_dir | items[selected].filename;
	}

	struct Item {
		Path filename;
		bool is_directory;
	};

	Path current_dir;
	Array<Item> items;
	string filter = "*";
	int hover = -1;
	int selected = -1;
};


/*
 *	Grid
 *		Grid
 *			Button up
 *			Label dir
 *		ListView
 *		Grid
 *			Label ""
 *			Button cancel
 *			Button ok
 */
FileSelectionDialog::FileSelectionDialog(Panel* parent, const string& title, const Path& dir, const Array<string>& params) : Dialog(title, 800, 600, parent) {

	saving = false;
	list = new FileSelectionControl("files");
	for (const auto& o: params)
		if (o.head(7) == "filter=")
			list->filter = o.sub(7);
	list->set_directory(dir);

	auto g1 = new Grid("grid1");
	add(g1);
	auto g2 = new Grid("grid2");
	g1->add(g2, 0, 0);
	auto button_up = new Button("up", "Up");
	button_up->expand_x = false;
	g2->add(button_up, 0, 0);
	auto x = new Label("directory", str(list->current_dir));
	x->expand_x = true;
	g2->add(x, 1, 0);
	g1->add(list, 0, 1);

	auto g3 = new Grid("grid3");
	g1->add(g3, 0, 2);
	auto spacer = new Label("spacer", "");
	g3->add(spacer, 0, 0);
	spacer->expand_x = true;
	auto button_cancel = new Button("cancel", "Cancel");
	button_cancel->min_width_user = 100;
	button_cancel->expand_x = false;
	g3->add(button_cancel, 1, 0);
	auto button_ok = new Button("ok", "Ok");
	button_ok->min_width_user = 100;
	button_ok->expand_x = false;
	button_ok->enable(false);
	g3->add(button_ok, 2, 0);

	event("up", [this] {
		list->set_directory(list->current_dir.parent());
		set_string("directory", str(list->current_dir));
	});
	event("ok", [this] {
		promise(list->get_selected_filename());
		request_destroy();
	});
	event("cancel", [this] {
		promise.fail();
		request_destroy();
	});
	event_x(list->id, "hui:change-directory", [this] {
		set_string("directory", str(list->current_dir));
	});
	event_x(list->id, event_id::Select, [this] {
		enable("ok", list->selected >= 0);
	});
}

base::future<Path> FileSelectionDialog::ask(Panel* parent, const string& title, const Path& dir, const Array<string>& params) {
	auto dlg = new FileSelectionDialog(parent, title, dir, params);
	return dlg->promise.get_future();
}



} // xhui