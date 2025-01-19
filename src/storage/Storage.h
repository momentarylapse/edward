/*
 * Storage.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_STORAGE_H_
#define SRC_STORAGE_STORAGE_H_

#include "format/Format.h"
#include "../lib/os/path.h"
#include "../lib/base/future.h"

struct ComplexPath {
	Path complete;
	Path relative;
	Path simple; //relative_no_extension
	int kind;
};

class Storage {
public:
	explicit Storage(Session *ed);
	~Storage();

	int guess_type(const Path &filename);

	bool load(const Path &filename, Data *data, bool deep = true);
	bool save(const Path &filename, Data *data);

	base::future<void> auto_save(Data *data);
	base::future<void> open(Data *data);
	base::future<void> save_as(Data *data);

	Session *session;
	Array<Format*> formats;


	static Path CANONICAL_SUB_DIR[NUM_FDS]; // relative


	void set_root_directory(const Path &directory, bool compact_mode = false);
	void guess_root_directory(const Path &filename);

	Path get_root_dir(int kind);
	string fd_ext(int kind);

	base::future<ComplexPath> file_dialog(int kind, bool save, bool force_in_root_dir);
	base::future<ComplexPath> file_dialog_x(const Array<int> &kind, int preferred, bool save, bool force_in_root_dir);

	Path root_dir;

	Path last_dir[NUM_FDS];
	Path root_dir_kind[NUM_FDS]; // absolute
	Array<Path> possible_sub_dir;
};

#endif /* SRC_STORAGE_STORAGE_H_ */
