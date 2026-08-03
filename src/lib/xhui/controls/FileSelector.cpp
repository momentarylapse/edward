//
// Created by Michael Ankele on 2025-02-04.
//

#include "FileSelector.h"
#include "Label.h"
#include "ListView.h"
#include "Button.h"
#include "../Application.h"
#include "../xhui.h"
#include <lib/base/iter.h>
#include <lib/image/image.h>
#include <lib/os/filesystem.h>
#include <lib/os/app.h>


namespace xhui {



class FileListView : public ListView {
public:
	explicit FileListView(const string& id) : ListView(id, "icon\\name") {
		show_headers = false;
		column_factories[0].f_create = [] (const string& _id) {
			return create_control("Image", "!padding=6", _id);
		};
		column_factories[0].f_set = [] (Control* c, const string& t) {
			c->set_option("image", t);
		};
		column_factories[1].f_create = [] (const string& _id) {
			return create_control("Label", "!padding=0", _id);
		};
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Expand;
	}
	void select_file(const Path& filename) {
		set_directory(filename.parent());
		for (const auto& [i, it]: enumerate(items)) {
			if (it.filename == filename.basename())
				set_int(i);
		}
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
		for (const auto& it: items) {
			string icon = get_file_icon(it.is_directory, it.filename.extension());
			add_string(format("%s\\%s", icon, it.filename.basename()));
		}
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

class DirectoryBar: public Grid {
public:
	Array<int> event_ids;
	Path directory;
	Path root;
	bool compact = false;
	DirectoryBar(const string& id) : Grid(id) {
		Grid::set_option("spacing", "4");
		root = os::app::home_directory;
	}

	void clear() {
		auto children = get_children(ChildFilter::All);
		for (auto c: children)
			Grid::remove_child(c);
		for (int e: event_ids)
			owner->remove_event_handler(e);
	}

	void set_directory(const Path& dir) {
		clear();

		directory = dir;

		Array<Path> parents;
		for (const auto& p: dir.all_parents())
			if (p.is_in(root))
				parents.add(p);
		bool cut_off = false;
		if (compact and parents.num >= 3) {
			parents = parents.sub_ref(0,2);
			cut_off = true;
		}
		int n = parents.num * 2;
		for (const auto& [i, p]: enumerate(parents)) {
			string id_up = format("%s:up:%d", id, i);
			string title = p.basename();
			if (cut_off and i == parents.num-1)
				title = "...";
			if (p == os::app::home_directory)
				title = "~";
			auto button_up = new Button(id_up, title);
			button_up->set_option("flat", "");
			button_up->set_option("bold", "false");
		//	button_up->set_option("url", "");
			button_up->set_option("paddingx", "2");
			button_up->set_option("paddingy", "3");
			button_up->set_option("tooltip", "Go to parent directory");
			button_up->size_mode_x = SizeMode::Shrink;
			Grid::add_child(button_up, n - i*2-2, 0);
			auto sep = new Label("", "/");
			sep->enable(false);
			Grid::add_child(sep, n-i*2-1, 0);

			event_ids.add(owner->event(id_up, [this, _p = p] {
				if (!_p.is_empty()) {
					directory = _p;
					emit_event(event_id::DirectoryChanged, true);
				}
			}));
		}

		auto directory_label = new Label(id + ":directory", directory.basename());
		if (directory == os::app::home_directory)
			directory_label->set_string("~");
		directory_label->size_mode_x = SizeMode::Expand;
		directory_label->ignore_hover = false;
		directory_label->set_option("bold", "");
		directory_label->set_option("tooltip", format("Current directory: %s", directory));
		Grid::add_child(directory_label, n, 0);
	}
};

/*
 *	Grid
 *		DirectoryBar
 *		FileListView
 *
 *	FileListView will emit event_id::DirectoryChanged (also for set_directory())
 *	-> update label etc on this event
 */
FileSelector::FileSelector(const string& id) : Grid(id) {
	directory_bar = new DirectoryBar(id + ":directory");
	Grid::add_child(directory_bar, 0, 0);
	list = new FileListView(id + ":list");
	Grid::add_child(list, 0, 1);
}

void FileSelector::link_events() {
	directory_bar->set_directory(current_directory());
	owner->event_x(directory_bar->id, event_id::DirectoryChanged, [this] {
		set_directory(directory_bar->directory);
	});
	owner->event_x(list->id, event_id::DirectoryChanged, [this] {
		update_directory_header();
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
	update_directory_header();
}

void FileSelector::update_directory_header() {
	directory_bar->set_directory(current_directory());
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
	if (key == "linkevents") {
		link_events();
	} else if (key == "directory") {
		set_directory(value);
	} else if (key == "select") {
		if (value != "") {
			set_directory(Path(value).parent());
			list->select_file(value);
		}
	} else if (key == "filter" or key == "dragsource") {
		list->set_option(key, value);
	} else if (key == "root") {
		root = value;
		directory_bar->root = root;
	} else if (key == "compact") {
		directory_bar->compact = true;
	} else if (key == "width") {
		directory_bar->compact = (value._int() < 400);
		Grid::set_option(key, value);
		list->set_option(key, value);
	} else {
		Grid::set_option(key, value);
	}
}

} // xhui