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

class FileSelector : public Grid {
public:
	explicit FileSelector(const string& id);
	void link_events();

	FileListView* list;
	Label* directory_label;

	void set_filter(const string& filter);
	void set_directory(const Path& dir);
	Path get_selected_filename() const;
	Path current_directory() const;
	void set_option(const string& key, const string& value) override;
};

} // xhui

#endif //FILESELECTOR_H
