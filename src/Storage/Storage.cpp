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
#include "../y/EngineData.h"

Storage *storage = nullptr;

Path Storage::CANONICAL_SUB_DIR[NUM_FDS];

Storage::Storage() {
	formats.add(new FormatFontX());
	formats.add(new FormatMaterial());
	formats.add(new FormatModel());
	formats.add(new FormatModelJson());
	formats.add(new FormatModel3ds());
	formats.add(new FormatModelPly());
	formats.add(new FormatTerrain());
	formats.add(new FormatWorld());

	CANONICAL_SUB_DIR[FD_FONT] = "Fonts";
	CANONICAL_SUB_DIR[FD_TERRAIN] = "Maps";
	CANONICAL_SUB_DIR[FD_WORLD] = "Maps";
	CANONICAL_SUB_DIR[FD_MODEL] = "Objects";
	CANONICAL_SUB_DIR[FD_MATERIAL] = "Materials";
	CANONICAL_SUB_DIR[FD_SHADERFILE] = "Materials";
	CANONICAL_SUB_DIR[FD_SOUND] = "Sounds";
	CANONICAL_SUB_DIR[FD_TEXTURE] = "Textures";
	CANONICAL_SUB_DIR[FD_SCRIPT] = "Scripts";

	dialog_file_kind = -1;
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

int Storage::guess_type(const Path &filename) {
	string ext = filename.extension();
	for (auto *f: formats) {
		if (f->extension == ext)
			return f->category;
	}
	return -1;
}

bool Storage::load(const Path &_filename, Data *data, bool deep) {
	auto filename = _filename.absolute().canonical();
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

			guess_root_directory(filename);
			data->reset();
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

bool Storage::save(const Path &_filename, Data *data) {
	auto filename = _filename.absolute().canonical();
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
	if (!file_dialog(type, false, false))
		return false;

	guess_root_directory(dialog_file_complete);

	try {
		return storage->load(dialog_file_complete, data);
	} catch(...) {
		return false;
	}
}

// canonical
bool Storage::save_as(Data *data) {
	int type = data_type(data);
	if (!file_dialog(type, true, false))
		return false;

	guess_root_directory(dialog_file_complete);

	try {
		return save(dialog_file_complete, data);
	} catch (...) {
		return false;
	}
}

bool Storage::auto_save(Data *data) {
	if (data->filename.is_empty())
		return save_as(data);
	return save(data->filename, data);
}



void Storage::guess_root_directory(const Path &filename) {
	for (auto &d: filename.all_parents())
		if (file_exists(d << "game.ini")) {
			set_root_directory(d);
			return;
		}

	set_root_directory(filename.parent(), true);
}


void Storage::set_root_directory(const Path &_directory, bool compact_mode) {
	Path directory = _directory.absolute().canonical();
	if (root_dir == directory)
		return;

	root_dir = directory;

	compact_mode = !file_exists(root_dir << "game.ini");


	for (int i=0; i<NUM_FDS; i++) {
		if (compact_mode)
			root_dir_kind[i] = root_dir;
		else
			root_dir_kind[i] = root_dir << CANONICAL_SUB_DIR[i];
		last_dir[i] = root_dir_kind[i];
	}

	engine.set_dirs(root_dir_kind[FD_TEXTURE],
			root_dir_kind[FD_WORLD],
			root_dir_kind[FD_MODEL],
			root_dir_kind[FD_SOUND],
			root_dir_kind[FD_SCRIPT],
			root_dir_kind[FD_MATERIAL],
			root_dir_kind[FD_FONT]);
}


Path Storage::get_root_dir(int kind) {
	if (kind==-1)
		return root_dir;
	return root_dir_kind[kind];
}

string fd_ext(int kind) {
	if (kind == FD_MODEL)
		return "model";
	if (kind == FD_TEXTURE)
		return "jpg,bmp,tga,png,avi";
	if (kind == FD_SOUND)
		return "wav,ogg";
	if (kind == FD_MATERIAL)
		return "material";
	if (kind == FD_TERRAIN)
		return "map";
	if (kind == FD_WORLD)
		return "world";
	if (kind == FD_SHADERFILE)
		return "shader";
	if (kind == FD_FONT)
		return "xfont";
	if (kind == FD_SCRIPT)
		return "kaba";
	if (kind == FD_CAMERAFLIGHT)
		return "camera";
	if (kind == FD_FILE)
		return "*";
	return "?";
}

string fd_name(int kind) {
	if (kind == FD_MODEL)
		return _("Model");
	if (kind == FD_TEXTURE)
		return _("Texture");
	if (kind == FD_SOUND)
		return _("Sound file");
	if (kind == FD_MATERIAL)
		return _("Material");
	if (kind == FD_TERRAIN)
		return _("Terrain");
	if (kind == FD_WORLD)
		return _("World");
	if (kind == FD_SHADERFILE)
		return _("Shader");
	if (kind == FD_FONT)
		return _("Font");
	if (kind == FD_SCRIPT)
		return _("Script");
	if (kind == FD_CAMERAFLIGHT)
		return _("Camera file");
	if (kind == FD_FILE)
		return _("arbitrary file");
	return "?";
}

bool Storage::file_dialog_x(const Array<int> &kind, int preferred, bool save, bool force_in_root_dir) {
	int done;

	string title, show_filter, filter;
	auto add_kind = [&] (const string &t, const string &sf, const string &f) {
		if (show_filter != "")
			show_filter += ", ";
		show_filter += sf;
		if (filter != "")
			filter += ";";
		filter += f;

		if (title == "")
			title = t;
	};
	auto ext2filter = [] (const string &ext) {
		auto xx = ext.explode(",");
		Array<string> ff;
		for (auto &x: xx)
			ff.add("*." + x);
		return implode(ff, ";");
	};
	for (auto k: kind) {
		add_kind(fd_name(k), format("%s (*.%s)", fd_name(k), fd_ext(k)), ext2filter(fd_ext(k)));
	}

	if (save)
		done = hui::FileDialogSave(ed, title, last_dir[preferred], show_filter, filter);
	else
		done = hui::FileDialogOpen(ed, title, last_dir[preferred], show_filter, filter);
	if (!done)
		return false;

	dialog_file_kind = FD_FILE;
	for (auto k: kind) {
		for (auto &ext: fd_ext(k).explode(","))
			if (hui::Filename.extension() == ext) {
				dialog_file_kind = k;
			}
	}


	bool in_root_dir = (hui::Filename.is_in(root_dir_kind[dialog_file_kind]));

	if (force_in_root_dir and !in_root_dir) {
		ed->error_box(hui::Filename.str());
		ed->error_box(format(_("The file is not in the appropriate directory: \"%s\"\nor in a subdirectory."), root_dir_kind[dialog_file_kind]));
		return false;
	}//else
		//MakeDirs(HuiFileDialogPath);

	if (in_root_dir)
		last_dir[dialog_file_kind] = hui::Filename.dirname();
	dialog_file_complete = hui::Filename;
	dialog_file = dialog_file_complete.relative_to(root_dir_kind[dialog_file_kind]);
	dialog_file_no_ending = dialog_file.no_ext();
	return true;
}

bool Storage::file_dialog(int kind, bool save, bool force_in_root_dir) {
	return file_dialog_x({kind}, kind, save, force_in_root_dir);
}
