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
#include "../lib/hui/Callback.h"



class Storage {
public:
	Storage();
	~Storage();

	int guess_type(const Path &filename);

	bool load(const Path &filename, Data *data, bool deep = true);
	bool save(const Path &filename, Data *data);

	hui::future<void> auto_save(Data *data);
	hui::future<void> open(Data *data);
	hui::future<void> save_as(Data *data);

	Array<Format*> formats;


	static Path CANONICAL_SUB_DIR[NUM_FDS]; // relative


	void set_root_directory(const Path &directory, bool compact_mode = false);
	void guess_root_directory(const Path &filename);

	Path get_root_dir(int kind);
	string fd_ext(int kind);

	hui::future<Path> file_dialog(int kind, bool save, bool force_in_root_dir);
	hui::future<Path> file_dialog_x(const Array<int> &kind, int preferred, bool save, bool force_in_root_dir);

	Path root_dir;

	Path last_dir[NUM_FDS];
	Path root_dir_kind[NUM_FDS]; // absolute
	Path dialog_file, dialog_file_complete, dialog_file_no_ending;
	int dialog_file_kind;
	Array<Path> possible_sub_dir;
};

extern Storage *storage;

#endif /* SRC_STORAGE_STORAGE_H_ */
