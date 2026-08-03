//
// Created by Michael Ankele on 2025-02-04.
//

#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include "Grid.h"
#include "../../os/path.h"

namespace xhui {

class Label;
class FileListView;
class DirectoryBar;

class FileSelector : public Grid {
public:
	explicit FileSelector(const string& id);
	void link_events();

	DirectoryBar* directory_bar;
	FileListView* list;
	Path root;

	string get_string() override;

	void set_filter(const string& filter);
	void set_directory(const Path& dir);
	void update_directory_header();
	Path get_selected_filename() const;
	Path current_directory() const;
	void set_option(const string& key, const string& value) override;
};

} // xhui

#endif //FILESELECTOR_H
