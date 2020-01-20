/*
 * Storage.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Storage.h"
#include "Format/FormatModel.h"
#include "../Edward.h"

Storage *storage = nullptr;

Storage::Storage() {
	formats.add(new FormatModel());
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
		if (f->category != type or f->extension != ext)
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
		if (f->category != type or f->extension != ext)
			continue;
		f->save(filename, data);
		return;
	}
	throw FormatUnhandledError();
}

