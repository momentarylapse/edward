//
// Created by Michael Ankele on 2025-02-04.
//

#include "FileSelector.h"
#include "Image.h"
#include "Label.h"
#include "ListView.h"
#include "Button.h"
#include "../Application.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../../base/iter.h"
#include "../../image/image.h"
#include "../../os/filesystem.h"
#include "../../os/app.h"
#include <cmath>

namespace xhui {


string create_rounded_icon(int N, const color& c) {
	const float r = 1.0f;
	const float R0 = (float)N / (float)sqrt(2.0) - r;
	::Image im(N, N, c);
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++) {
			vec2 d = vec2((float)i - (float)N/2 + 0.5f, (float)j - (float)N/2 + 0.5f);
			im.set_pixel(i, j, c.with_alpha(clamp(R0 - d.length(), 0.0f, 1.0f)));
		}
	return create_image(im);
}

class FileListView : public ListView {
public:
	string icon_dir;
	string icon_file;
	explicit FileListView(const string& id) : ListView(id, "icon\\name") {
		show_headers = false;
		icon_dir = create_rounded_icon(16, color(1, 0.8f, 0.6f, 0));
		icon_file = create_rounded_icon(16, Theme::_default.text_label);
		column_factories[0].f_create = [] (const string& _id) {
			return new Image(_id, "");
		};
		column_factories[0].f_set = [this] (Control* c, const string& t) {
			c->set_option("image", (t == "d") ? icon_dir : icon_file);
		};
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Expand;
	}
	void set_directory(const Path& dir) {
		current_dir = dir;
		if (dir.is_empty())
			current_dir = os::app::initial_working_directory;
		update_files();
		emit_event(event_id::DirectoryChanged, false);
	}
	void set_filter(const string& _filter) {
		filter = _filter.explode(";");
		update_files();
	}
	void update_files() {
		items.clear();
		reset();
		auto ff = [this] (const Path& fn) {
			for (const auto& _f: filter)
				if (str(fn).match(_f))
					return true;
			return filter.num == 0;
		};
		const auto list = os::fs::search(current_dir, "*", "df");
		for (const auto& e: list)
			if (str(e).head(1) != ".") {
				if (os::fs::is_directory(current_dir | e))
					items.add({e, true});
				else if (ff(e))
					items.add({e, false});
			}
		for (const auto& it: items)
			add_string(format("%s\\%s", it.is_directory ? "d" : "f", it.filename.basename()));
		request_redraw();
		emit_event(event_id::Select, false);
	}
	void on_click_row(int row) override {
		const auto e = items[row];
		if (e.is_directory) {
			set_directory(current_dir | e.filename);
			emit_event(event_id::DirectoryChanged, false);
		}
	}
	void on_double_click_row(int row) override {
		const auto e = items[row];
		if (e.is_directory) {
			set_directory(current_dir | e.filename);
			emit_event(event_id::DirectoryChanged, false);
		} else {
			emit_event(event_id::Activate, false);
		}
	}
	Path get_selected_filename() const {
		if (selected.num == 0)
			return "";
		return current_dir | items[selected[0]].filename;
	}
	void set_option(const string& key, const string& value) override {
		if (key == "directory")
			set_directory(value);
		else if (key == "filter")
			set_filter(value);
		else if (key == "dragsource")
			drag_source_id = value;
		else
			ListView::set_option(key, value);
	}

	struct Item {
		Path filename;
		bool is_directory;
	};

	Path current_dir;
	Array<Item> items;
	Array<string> filter;
	string drag_source_id;
};

/*
 *	Grid
 *		Grid
 *			Button up
 *			Label dir
 *		FileListView
 *
 *	FileListView will emit event_id::DirectoryChanged (also for set_directory())
 *	-> update label etc on this event
 */
FileSelector::FileSelector(const string& id) : Grid(id) {
	auto g = new Grid(id + ":group");
	Grid::add_child(g, 0, 0);
	auto button_up = new Button(id+":up", "Up");
	button_up->size_mode_x = SizeMode::Shrink;
	g->add_child(button_up, 0, 0);
	directory_label = new Label(id + ":directory", "");
	directory_label->size_mode_x = SizeMode::Expand;
	g->add_child(directory_label, 1, 0);
	list = new FileListView(id + ":list");
	Grid::add_child(list, 0, 1);

	directory_label->set_string(str(current_directory().basename()));
}

void FileSelector::link_events() {
	owner->event(id + ":up", [this] {
		if (!current_directory().parent().is_empty())
			set_directory(current_directory().parent());
	});
	owner->event_x(list->id, event_id::DirectoryChanged, [this] {
		//directory_label->set_string(str(current_directory()));
		directory_label->set_string(str(current_directory().basename()));
		owner->enable(id + ":up", (!root or current_directory() != root));
	});
	owner->event_x(list->id, event_id::Select, [this] {
		emit_event(event_id::Select, false);
	});
	owner->event_x(list->id, event_id::Activate, [this] {
		if (!emit_event(event_id::Activate, true))
			emit_event(event_id::ActivateDialogDefault, false);
	});
	owner->event_x(list->id, event_id::DragStart, [this] {
		owner->get_window()->start_drag(get_selected_filename().basename(), "filename:" + str(get_selected_filename()));
	});
}

void FileSelector::set_filter(const string& filter) {
	list->set_filter(filter);
}

void FileSelector::set_directory(const Path& dir) {
	if (root and !dir.is_in(root))
		return;
	list->set_directory(dir);
}

Path FileSelector::get_selected_filename() const {
	return list->get_selected_filename();
}

Path FileSelector::current_directory() const {
	return list->current_dir;
}

string FileSelector::get_string() {
	return str(get_selected_filename());
}

void FileSelector::set_option(const string& key, const string& value) {
	if (key == "linkevents")
		link_events();
	else if (key == "directory")
		set_directory(value);
	else if (key == "filter" or key == "dragsource")
		list->set_option(key, value);
	else if (key == "root")
		root = value;
	else
		Grid::set_option(key, value);
}

} // xhui