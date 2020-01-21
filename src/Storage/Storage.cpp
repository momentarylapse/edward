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
#include "../meta.h"

Storage *storage = nullptr;

static string CANONICAL_SUB_DIR[NUM_FDS];

Storage::Storage() {
	formats.add(new FormatFontX());
	formats.add(new FormatMaterial());
	formats.add(new FormatModel());
	formats.add(new FormatModelJson());
	formats.add(new FormatModel3ds());
	formats.add(new FormatModelPly());
	formats.add(new FormatTerrain());
	formats.add(new FormatWorld());

	CANONICAL_SUB_DIR[FD_FONT] = "Fonts/";
	CANONICAL_SUB_DIR[FD_TERRAIN] = "Maps/";
	CANONICAL_SUB_DIR[FD_WORLD] = "Maps/";
	CANONICAL_SUB_DIR[FD_MODEL] = "Objects/";
	CANONICAL_SUB_DIR[FD_MATERIAL] = "Materials/";
	CANONICAL_SUB_DIR[FD_SHADERFILE] = "Materials/";
	CANONICAL_SUB_DIR[FD_SOUND] = "Sounds/";
	CANONICAL_SUB_DIR[FD_TEXTURE] = "Textures/";
	CANONICAL_SUB_DIR[FD_SCRIPT] = "Scripts/";
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

			guess_root_directory(filename);
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
	if (data->filename == "")
		return save_as(data);
	return save(data->filename, data);
}




string canonical(const string &d) {
	if (d.head(1) == "/")
		return d.no_recursion();
	return (get_current_dir() + d).no_recursion();
}

Array<string> parent_dirs(const string &_filename) {
	Array<string> dirs;
	string filename = canonical(_filename);
	for (int i=filename.num-1; i--; i>=1) {
		if (filename[i] == '/')
			dirs.add(filename.substr(0, i+1));
	}
	return dirs;
}

void Storage::guess_root_directory(const string &filename) {
	for (auto &d: parent_dirs(filename))
		if (file_test_existence(d + "game.ini")) {
			set_root_directory(d);
			return;
		}

	set_root_directory(filename.dirname(), true);
}


void Storage::set_root_directory(const string &_directory, bool compact_mode) {
	string directory = canonical(_directory);
	directory.dir_ensure_ending();
	if (root_dir == directory)
		return;

	root_dir = directory;

	compact_mode = !file_test_existence(root_dir + "game.ini");


	for (int i=0; i<NUM_FDS; i++) {
		if (compact_mode)
			root_dir_kind[i] = root_dir;
		else
			root_dir_kind[i] = root_dir + CANONICAL_SUB_DIR[i];
		dialog_dir[i] = root_dir_kind[i];
	}

	MetaSetDirs(root_dir_kind[FD_TEXTURE],
			root_dir_kind[FD_WORLD],
			root_dir_kind[FD_MODEL],
			root_dir_kind[FD_SOUND],
			root_dir_kind[FD_SCRIPT],
			root_dir_kind[FD_MATERIAL],
			root_dir_kind[FD_FONT]);
}


string Storage::get_root_dir(int kind) {
	if (kind==-1)
		return root_dir;
	return root_dir_kind[kind];
}

static string no_extension(const string &filename) {
	int p = filename.rfind(".");
	if (p >= 0)
		return filename.substr(0, p);
	return filename;
}

bool Storage::file_dialog(int kind, bool save, bool force_in_root_dir)
{
	int done;

	string title, show_filter, filter;
	if (kind==FD_MODEL){		title=_("Model file");	show_filter=_("Models (*.model)");			filter="*.model";	}
	if (kind==FD_TEXTURE){	title=_("Texture file");	show_filter=_("Textures (bmp,jpg,tga,png,avi)");filter="*.jpg;*.bmp;*.tga;*.png;*.avi";	}
	if (kind==FD_SOUND){		title=_("Sound file");		show_filter=_("Sounds (wav,ogg)");			filter="*.wav;*.ogg";	}
	if (kind==FD_MATERIAL){	title=_("Material file");	show_filter=_("Materials (*.material)");	filter="*.material";	}
	if (kind==FD_TERRAIN){	title=_("Terrain files");	show_filter=_("Terrains (*.map)");			filter="*.map";	}
	if (kind==FD_WORLD){		title=_("World file");		show_filter=_("Worlds (*.world)");			filter="*.world";	}
	if (kind==FD_SHADERFILE){title=_("Shader file");	show_filter=_("Shader files (*.shader)");	filter="*.shader";	}
	if (kind==FD_FONT){		title=_("Font file");		show_filter=_("Font files (*.xfont)");	filter="*.xfont";	}
	if (kind==FD_SCRIPT){	title=_("Script file");	show_filter=_("Script files (*.kaba)");	filter="*.kaba";	}
	if (kind==FD_CAMERAFLIGHT){title=_("Camera file");	show_filter=_("Camera files (*.camera)");	filter="*.camera";	}
	if (kind==FD_FILE){		title=_("arbitrary file");	show_filter=_("Files (*.*)");				filter="*";	}

	if (save)
		done = hui::FileDialogSave(ed, title, dialog_dir[kind], show_filter, filter);
	else
		done = hui::FileDialogOpen(ed, title, dialog_dir[kind], show_filter, filter);
	if (done) {

		bool in_root_dir = (hui::Filename.sys_filename().find(root_dir_kind[kind].sys_filename()) >= 0);

		if (force_in_root_dir) {
			if (!in_root_dir) {
				ed->error_box(hui::Filename.sys_filename());
				ed->error_box(format(_("The file is not in the appropriate directory: \"%s\"\nor in a subdirectory."), root_dir_kind[kind].sys_filename().c_str()));
				return false;
			}
		}//else
			//MakeDirs(HuiFileDialogPath);

		if (in_root_dir)
			dialog_dir[kind] = hui::Filename.dirname();
		dialog_file_complete = hui::Filename;
		dialog_file = dialog_file_complete.substr(root_dir_kind[kind].num, -1);
		dialog_file_no_ending = no_extension(dialog_file);

		return true;
	}
	return false;
}
