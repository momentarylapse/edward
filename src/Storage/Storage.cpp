/*
 * Storage.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Storage.h"
#include "Format/FormatModel.h"
#include "Format/FormatModelJson.h"
#include "../Edward.h"

Storage *storage = nullptr;

Storage::Storage() {
	formats.add(new FormatModel());
	formats.add(new FormatModelJson());
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
		if ((int)f->flags & (int)Format::Flag::LOAD == 0)
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
		if ((int)f->flags & (int)Format::Flag::SAVE == 0)
			continue;
		f->save(filename, data);
		return;
	}
	throw FormatUnhandledError();
}

