/*
 * Storage.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Storage.h"
#include "format/FormatFontX.h"
#include "format/FormatMaterial.h"
#include "format/FormatModel.h"
#include "format/FormatModelJson.h"
#include "format/FormatModel3ds.h"
#include "format/FormatModelPly.h"
#include "format/FormatTerrain.h"
#include "format/FormatWorld.h"
#include "../lib/os/filesystem.h"
#include "../lib/os/msg.h"
#include "../lib/hui/hui.h"
#include <y/EngineData.h>
#include "../Session.h"
#include "../EdwardWindow.h"


Path Storage::CANONICAL_SUB_DIR[NUM_FDS];

Storage::Storage(Session *_s) {
	session = _s;
	formats.add(new FormatFontX(session));
	formats.add(new FormatMaterial(session));
	formats.add(new FormatModel(session));
	formats.add(new FormatModelJson(session));
	formats.add(new FormatModel3ds(session));
	formats.add(new FormatModelPly(session));
	formats.add(new FormatTerrain(session));
	formats.add(new FormatWorld(session));

	CANONICAL_SUB_DIR[FD_FONT] = "Fonts";
	CANONICAL_SUB_DIR[FD_TERRAIN] = "Maps";
	CANONICAL_SUB_DIR[FD_WORLD] = "Maps";
	CANONICAL_SUB_DIR[FD_MODEL] = "Objects";
	CANONICAL_SUB_DIR[FD_MATERIAL] = "Materials";
	CANONICAL_SUB_DIR[FD_SHADERFILE] = "Materials";
	CANONICAL_SUB_DIR[FD_SOUND] = "Sounds";
	CANONICAL_SUB_DIR[FD_TEXTURE] = "Textures";
	CANONICAL_SUB_DIR[FD_SCRIPT] = "Scripts";
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
			data->out_changed();

			return true;
		}
		throw FormatUnhandledError();
	} catch (Exception &e) {
		if (session)
			session->error(e.message());
		else
			msg_error(e.message());
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
		session->error(e.message());
	}
	return false;
}

// canonical
base::future<void> Storage::open(Data *data) {
	base::promise<void> promise;

	session->allow_termination().then([this, data, promise] {
		int type = data_type(data);
		file_dialog(type, false, false).then([this, data, promise] (const auto& p) mutable {
			guess_root_directory(p.complete);

			try {
				if (this->load(p.complete, data))
					promise();
			} catch(...) {
				promise.fail();
			}
		}).on_fail([promise] () mutable {
			promise.fail();
		});
	}).on_fail([promise] () mutable {
		promise.fail();
	});

	return promise.get_future();
}

// canonical
base::future<void> Storage::save_as(Data *data) {
	base::promise<void> promise;

	int type = data_type(data);
	file_dialog(type, true, false).then([this, data, promise] (const auto& p) mutable {
		guess_root_directory(p.complete);

		try {
			if (save(p.complete, data))
				promise();
			else
				promise.fail();
		} catch (...) {
			promise.fail();
		}
	}).on_fail([promise] () mutable {
		promise.fail();
	});
	return promise.get_future();
}

base::future<void> Storage::auto_save(Data *data) {
	if (data->filename) {
		base::promise<void> promise;
		if (save(data->filename, data))
			promise();
		else
			promise.fail();
		return promise.get_future();
	} else {
		return save_as(data);
	}
}



void Storage::guess_root_directory(const Path &filename) {
	for (auto &d: filename.all_parents())
		if (os::fs::exists(d | "game.ini")) {
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

	compact_mode = !os::fs::exists(root_dir | "game.ini");


	for (int i=0; i<NUM_FDS; i++) {
		if (compact_mode)
			root_dir_kind[i] = root_dir;
		else
			root_dir_kind[i] = root_dir | CANONICAL_SUB_DIR[i];
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

string Storage::fd_ext(int kind) {
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

base::future<ComplexPath> Storage::file_dialog_x(const Array<int> &kind, int preferred, bool save, bool force_in_root_dir) {
	int done;
	//static hui::promise<ComplexPath> promise;
	//promise.reset();
	base::promise<ComplexPath> promise;

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


	auto on_select_base = [this, kind, force_in_root_dir, promise] (const Path &path) mutable {
		ComplexPath cp;
		cp.kind = FD_FILE;
		for (auto k: kind) {
			for (auto &ext: fd_ext(k).explode(","))
				if (path.extension() == ext) {
					cp.kind = k;
				}
		}


		bool in_root_dir = (path.is_in(root_dir_kind[cp.kind]));

		if (force_in_root_dir and !in_root_dir) {
			session->error(path.str());
			session->error(format(_("The file is not in the appropriate directory: \"%s\"\nor in a subdirectory."), root_dir_kind[cp.kind]));
			return;
		}//else
			//MakeDirs(HuiFileDialogPath);

		if (in_root_dir)
			last_dir[cp.kind] = path.dirname();
		cp.complete = path;
		cp.relative = path.relative_to(root_dir_kind[cp.kind]);
		cp.simple = cp.relative.no_ext();
		promise(cp);
	};

	if (save)
		hui::file_dialog_save(session->win, title, last_dir[preferred], {"showfilter="+show_filter, "filter="+filter})
			.then(on_select_base)
			.on_fail([promise] () mutable { promise.fail(); });
	else
		hui::file_dialog_open(session->win, title, last_dir[preferred], {"showfilter="+show_filter, "filter="+filter})
			.then(on_select_base)
			.on_fail([promise] () mutable { promise.fail(); });

	return promise.get_future();
}

base::future<ComplexPath> Storage::file_dialog(int kind, bool save, bool force_in_root_dir) {
	return file_dialog_x({kind}, kind, save, force_in_root_dir);
}
