/*
 * Storage.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_STORAGE_H_
#define SRC_STORAGE_STORAGE_H_

#include "Format/Format.h"



class Storage {
public:
	Storage();
	~Storage();

	bool load(const string &filename, Data *data, bool deep = true);
	bool save(const string &filename, Data *data);
	bool auto_save(Data *data);

	bool open(Data *data);
	bool save_as(Data *data);

	Array<Format*> formats;




	void set_root_directory(const string &directory, bool compact_mode = false);
	void guess_root_directory(const string &filename);

	string get_root_dir(int kind);

	bool file_dialog(int kind, bool save, bool force_in_root_dir);

	string root_dir;

	string dialog_dir[NUM_FDS], root_dir_kind[NUM_FDS];
	string dialog_file, dialog_file_complete, dialog_file_no_ending;
	Array<string> possible_sub_dir;
};

extern Storage *storage;

#endif /* SRC_STORAGE_STORAGE_H_ */