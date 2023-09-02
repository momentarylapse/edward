/*
 * ConfigurationDialog.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ConfigurationDialog.h"
#include "../../../data/administration/DataAdministration.h"
#include "../../../data/administration/GameIniData.h"
#include "../../../Edward.h"
#include "../../../storage/Storage.h"

ConfigurationDialog::ConfigurationDialog(hui::Window* _parent, DataAdministration *_data, bool _exporting):
	hui::Dialog(_exporting ? "ge_dialog" : "game-config-dialog", 400, 300, _parent, false)
{
	from_resource(_exporting ? "ge_dialog" : "game-config-dialog");
	exporting = _exporting;
	data = _data;

	// dialog
	event("hui:close", [this] { on_close(); });
	event("cancel", [this] { on_close(); });
	event("ok", [this] { on_ok(); });
	//event("find-rootdir", [this] { OnFindRootdir(); });
	event("find-world", [this] { on_find_world(); });
	event("find-second-world", [this] { on_find_second_world(); });
	event("find-script", [this] { on_find_script(); });
	event("find-material", [this] { on_find_material(); });
	event("find-font", [this] { on_find_font(); });

	load_data();
}

static const Array<string> WINDOW_MODES = {"windowed", "windowed-fullscreen", "fullscreen"};
static const Array<string> RENDER_PATHS = {"direct", "forward", "deferred", "raytracing"};
static const Array<int> SHADOW_RESOLUTIONS = {512, 1024, 2048, 4096};

void ConfigurationDialog::load_data() {
	if (exporting){
		#ifdef HUI_OS_WINDOWS
			setInt("ged_system", 1);
		#endif
		set_int("export_type",0);
	}

	GameIniData &game = *data->GameIni;
	game.load(storage->root_dir);
	set_string("world", game.default_world().str());
	set_string("second-world", game.second_world().str());
	set_string("script", game.default_script().str());
	set_string("material", game.default_material().str());
	set_string("font", game.default_font().str());

	set_int("render-path", max(0, RENDER_PATHS.find(game.get_str(game.ID_RENDER_PATH, "forward"))));
	set_int("mode", max(0, WINDOW_MODES.find(game.get_str(game.ID_SCREEN_MODE, "windowed"))));
	check("hdr", true);
	set_int("fps-target", game.get_int(game.ID_RENDERER_FRAMERATE, 60));
	set_float("scale-min", game.get_float(game.ID_RESOLUTION_SCALE_MIN, 1.0f));
	set_int("shadow-resolution", max(0, SHADOW_RESOLUTIONS.find(game.get_int(game.ID_SHADOW_RESOLUTION, 2048))));

	set_string("root-directory", storage->root_dir.str());
}


void ConfigurationDialog::on_find_root_dir() {
	hui::file_dialog_dir(this, _("Working directory"), storage->root_dir, {}).on([this] (const Path &path) {
		set_string("root-directory", path.str());
	});
}

void ConfigurationDialog::on_find_world() {
	storage->file_dialog(FD_WORLD, false, true).on([this] (const auto& p) {
		set_string("world", p.simple.str());
	});
}

void ConfigurationDialog::on_find_second_world() {
	storage->file_dialog(FD_WORLD, false, true).on([this] (const auto& p) {
		set_string("second-world", p.simple.str());
	});
}

void ConfigurationDialog::on_find_script() {
	storage->file_dialog(FD_SCRIPT, false, true).on([this] (const auto& p) {
		set_string("script", p.relative.str());
	});
}

void ConfigurationDialog::on_find_material() {
	storage->file_dialog(FD_MATERIAL, false, true).on([this] (const auto& p) {
		set_string("material", p.simple.str());
	});
}

void ConfigurationDialog::on_find_font() {
	storage->file_dialog(FD_FONT, false, true).on([this] (const auto& p) {
		set_string("font", p.simple.str());
	});
}

void ConfigurationDialog::into_game_init(GameIniData &g) {
	g.set_str(g.ID_WORLD, get_string("world"));
	g.set_str(g.ID_WORLD2, get_string("second-world"));
	g.set_str(g.ID_SCRIPT, get_string("script"));
	g.set_str(g.ID_MATERIAL, get_string("material"));
	g.set_str(g.ID_FONT, get_string("font"));

	g.set_str(g.ID_SCREEN_MODE, WINDOW_MODES[get_int("mode")]);
	g.set_str(g.ID_RENDER_PATH, RENDER_PATHS[get_int("render-path")]);
	g.set_int(g.ID_RENDERER_FRAMERATE, get_int("fps-target"));
	g.set_float(g.ID_RESOLUTION_SCALE_MIN, get_float("scale-min"));
	g.set_int(g.ID_SHADOW_RESOLUTION, SHADOW_RESOLUTIONS[get_int("shadow-resolution")]);
}

void ConfigurationDialog::on_ok() {
	if (exporting) {
		//GameIniAlt=GameIni;
		GameIniData GameIniExport = *data->GameIni;
		string dir = get_string("root-directory");
		into_game_init(GameIniExport);
//		_exporting_type_ = getInt("export_type");
//		_exporting_system_ = getInt("ged_system");
		try {
			data->ExportGame(dir, GameIniExport);
		} catch(AdminGameExportException &e) {
			ed->error_box(_("Error while exporting:") + e.message);
			return;
		}
	} else {
		// new RootDir?
		bool rdc = (storage->root_dir != get_string("root-directory"));
		if (rdc)
			storage->set_root_directory(get_string("root-directory"));
//		data->UnlinkGameIni();
		into_game_init(*data->GameIni);
//		data->LinkGameIni(data->GameIni);
		data->GameIni->save(storage->root_dir);
		if (rdc)
			data->UpdateDatabase();
	}
	exporting = false;
	request_destroy();
}

void ConfigurationDialog::on_close() {
	request_destroy();
}
