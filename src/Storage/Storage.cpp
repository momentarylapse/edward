/*
 * Storage.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Storage.h"
#include "Format/FormatFontX.h"
#include "Format/FormatMaterial.h"
#include "Format/FormatModel.h"
#include "Format/FormatModelJson.h"
#include "Format/FormatModel3ds.h"
#include "Format/FormatModelPly.h"
#include "Format/FormatTerrain.h"
#include "Format/FormatWorld.h"
#include "../Edward.h"

Storage *storage = nullptr;

Storage::Storage() {
	formats.add(new FormatFontX());
	formats.add(new FormatMaterial());
	formats.add(new FormatModel());
	formats.add(new FormatModelJson());
	formats.add(new FormatModel3ds());
	formats.add(new FormatModelPly());
	formats.add(new FormatTerrain());
	formats.add(new FormatWorld());
}

Storage::~Storage() {
	for (auto *f: formats)
		delete f;
}

int data_type(Data *data) {
	return data->type;
	/*if (dynamic_cast<DataModel*>(data))
		return FD_MODEL;
	return -1;*/
}

bool Storage::load(const string &filename, Data *data, bool deep) {
	try {
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
			data->filename = filename;
			data->reset_history();
			data->notify();

			return true;
		}
		throw FormatUnhandledError();
	} catch (Exception &e) {
		ed->error_box(e.message());
	}
	return false;
}

bool Storage::save(const string &filename, Data *data) {
	try {
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

			data->filename = filename;
			data->action_manager->mark_current_as_save();
			return true;
		}
		throw FormatUnhandledError();
	} catch (Exception &e) {
		ed->error_box(e.message());
	}
	return false;
}

// canonical
bool Storage::open(Data *data) {
	if (!ed->allow_termination())
		return false;

	int type = data_type(data);
	if (!ed->file_dialog(type, false, false))
		return false;

	ed->make_dirs(ed->dialog_file_complete);

	try {
		return storage->load(ed->dialog_file_complete, data);
	} catch(...) {
		return false;
	}
}

// canonical
bool Storage::save_as(Data *data) {
	int type = data_type(data);
	if (!ed->file_dialog(type, true, false))
		return false;

	ed->make_dirs(ed->dialog_file_complete);

	try {
		return save(ed->dialog_file_complete, data);
	} catch (...) {
		return false;
	}
}

bool Storage::auto_save(Data *data) {
	if (data->filename == "")
		return save_as(data);
	return save(data->filename, data);
}
