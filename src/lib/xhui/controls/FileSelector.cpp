//
// Created by Michael Ankele on 2025-02-04.
//

#include "FileSelector.h"
#include "Label.h"
#include "Button.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../../lib/base/iter.h"
//#include "../../lib/os/msg.h"
#include "../../os/filesystem.h"

namespace xhui {


class FileListView : public Control {
public:
	explicit FileListView(const string& id) : Control(id) {
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Expand;
	}
	void set_directory(const Path& dir) {
		current_dir = dir;
		if (dir.is_empty())
			current_dir = Application::initial_working_directory;
		update_files();
	}
	void set_filter(const string& _filter) {
		filter = _filter;
		update_files();
	}
	void update_files() {
		hover = -1;
		selected = -1;
		items.clear();
		const auto list = os::fs::search(current_dir, filter, "df");
		for (const auto& e: list)
			if (str(e).head(1) != ".")
				items.add({e, os::fs::is_directory(current_dir | e)});
		request_redraw();
		emit_event(event_id::Select, false);
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
			if (drag_source_id != "")
				owner->get_window()->start_pre_drag(this);
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
	void set_option(const string& key, const string& value) override {
		if (key == "directory")
			set_directory(value);
		else if (key == "filter")
			set_filter(value);
		else if (key == "dragsource")
			drag_source_id = value;
		else
			Control::set_option(key, value);
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
	string drag_source_id;
};

/*
 *	Grid
 *		Grid
 *			Button up
 *			Label dir
 *		ListView
 */
FileSelector::FileSelector(const string& id) : Grid(id) {
	auto g = new Grid(id + ":group");
	Grid::add(g, 0, 0);
	auto button_up = new Button(id+":up", "Up");
	button_up->size_mode_x = SizeMode::Shrink;
	g->add(button_up, 0, 0);
	directory_label = new Label(id + ":directory", "");
	directory_label->size_mode_x = SizeMode::Expand;
	g->add(directory_label, 1, 0);
	list = new FileListView(id + ":list");
	Grid::add(list, 0, 1);

	directory_label->set_string(str(current_directory().basename()));
}
void FileSelector::link_events() {
	owner->event(id + ":up", [this] {
		if (!current_directory().parent().is_empty())
			set_directory(current_directory().parent());
	});
	owner->event_x(list->id, "hui:change-directory", [this] {
		//directory_label->set_string(str(current_directory()));
			directory_label->set_string(str(current_directory().basename()));
	});
	owner->event_x(list->id, event_id::Select, [this] {
		emit_event(event_id::Select, false);
	});
	owner->event_x(list->id, event_id::DragStart, [this] {
		owner->get_window()->start_drag(get_selected_filename().basename(), "filename:" + str(get_selected_filename()));
	});
}

void FileSelector::set_filter(const string& filter) {
	list->set_filter(filter);
}
void FileSelector::set_directory(const Path& dir) {
	list->set_directory(dir);
	directory_label->set_string(str(current_directory().basename()));
}
Path FileSelector::get_selected_filename() const {
	return list->get_selected_filename();
}
Path FileSelector::current_directory() const {
	return list->current_dir;
}
void FileSelector::set_option(const string& key, const string& value) {
	if (key == "linkevents")
		link_events();
	else if (key == "directory" or key == "filter" or key == "dragsource")
		list->set_option(key, value);
	else
		Grid::set_option(key, value);
}

} // xhui