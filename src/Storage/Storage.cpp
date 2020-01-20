/*
 * Storage.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Storage.h"
#include "Format/FormatModel.h"
#include "Format/FormatModelJson.h"
#include "Format/FormatModel3ds.h"
#include "Format/FormatModelPly.h"
#include "../Edward.h"

Storage *storage = nullptr;

Storage::Storage() {
	formats.add(new FormatModel());
	formats.add(new FormatModelJson());
	formats.add(new FormatModel3ds());
	formats.add(new FormatModelPly());
}

Storage::~Storage() {
	for (auto *f: formats)
		delete f;
}

int data_type(Data *data) {
	if (dynamic_cast<DataModel*>(data))
		return FD_MODEL;
	return -1;
}

void Storage::load(const string &filename, Data *data, bool deep) {
	int type = data_type(data);
	string ext = filename.extension();
	for (auto *f: formats) {
		if (f->category != type)
			continue;
		if (f->extension != ext)
			continue;
		if ((int)f->flags & (int)Format::Flag::READ == 0)
			continue;
		f->load(filename, data, deep);
		return;
	}
	throw FormatUnhandledError();
}

void Storage::save(const string &filename, Data *data) {
	int type = data_type(data);
	string ext = filename.extension();
	for (auto *f: formats) {
		if (f->category != type)
			continue;
		if (f->extension != ext)
			continue;
		if ((int)f->flags & (int)Format::Flag::READ == 0)
			continue;
		f->save(filename, data);
		return;
	}
	throw FormatUnhandledError();
}

bool Storage::open(Data *data) {
	if (!ed->file_dialog(FD_FILE, true, false))
		return false;

	try {
		storage->load(ed->dialog_file_complete, data);
		return true;
	} catch(...) {
		return false;
	}
}

bool Storage::save_as(Data *data) {
	if (!ed->file_dialog(FD_FILE, true, false))
		return false;
	try {
		save(ed->dialog_file_complete, data);
		return true;
	} catch (...) {
		return false;
	}
}
