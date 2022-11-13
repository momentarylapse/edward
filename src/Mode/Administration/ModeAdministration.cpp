/*
 * ModeAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeAdministration.h"
#include "Dialog/AdministrationDialog.h"
#include "Dialog/ConfigurationDialog.h"
#include "Dialog/NewProjectDialog.h"
#include "../../lib/os/filesystem.h"
#include "../../lib/os/file.h"
#include "../../lib/os/terminal.h"
#include "../../Data/Administration/DataAdministration.h"
#include "../../Data/Administration/GameIniData.h"
#include "../../Data/World/DataWorld.h"
#include "../../Edward.h"
#include "../../Storage/Storage.h"

ModeAdministration::ModeAdministration():
	Mode<DataAdministration>("Administration", nullptr, new DataAdministration, nullptr, "menu_administration")
{
	dialog = nullptr;
}

ModeAdministration::~ModeAdministration() {
}

void ModeAdministration::on_start() {
	data->LoadDatabase();
	data->UpdateDatabase();
	dialog = new AdministrationDialog(ed, true, data);
	dialog->show();
}

void ModeAdministration::on_end() {
	delete dialog;
}

void ModeAdministration::on_command(const string& id) {
	if (id == "export_game")
		export_game();
	if (id == "rudimentary_configuration")
		basic_settings();
}

void ModeAdministration::basic_settings() {
	hui::fly(new ConfigurationDialog(hui::CurWindow, data, false));
}

void ModeAdministration::export_game() {
	hui::fly(new ConfigurationDialog(hui::CurWindow, data, true));
	// data->ExportGame(...);
}


void ModeAdministration::create_project(const Path &dir, const string &first_world) {
	upgrade_project(dir);

	GameIniData gi;
	gi.load(dir);
	gi.set_str(GameIniData::ID_WORLD, first_world);
	gi.save(dir);

	Path world_file = dir << "Maps" << (first_world + ".world");
	msg_write(format("%sCREATE%s  %s", os::terminal::YELLOW, os::terminal::END, world_file));
	DataWorld w;
	Storage s;
	s.save(world_file, &w);
}

static void create_directory_recursive(const Path &dir) {
	if (dir.is_empty())
		return;
	if (dir.parent() != dir)
		if (!os::fs::exists(dir.parent()))
			create_directory_recursive(dir.parent());
	if (!os::fs::exists(dir)) {
		os::fs::create_directory(dir);
		msg_write(format("%sCREATE%s  %s", os::terminal::YELLOW, os::terminal::END, dir));
	}
}

static void sync_files(const Path &source, const Path &dest) {
	create_directory_recursive(dest.parent());
	if (os::fs::exists(dest)) {
		if (os::fs::read_binary(dest) == os::fs::read_binary(source)) {
			msg_write(format("%sOK%s  %s", os::terminal::GREEN, os::terminal::END, dest));
			return;
		}
	}
	msg_write(format("%sCOPY%s  %s", os::terminal::YELLOW, os::terminal::END, dest));
	os::fs::copy(source, dest);
}

void ModeAdministration::upgrade_project(const Path &dir) {
	Storage s; // create CANONICAL_SUB_DIR[]

	for (int k=0; k<NUM_FDS; k++)
		create_directory_recursive(dir << Storage::CANONICAL_SUB_DIR[k]);

	if (os::fs::exists(dir << "game.ini")) {
		msg_write(format("%sOK%s  %s", os::terminal::GREEN, os::terminal::END, (dir << "game.ini")));
	} else {
		msg_write(format("%sCREATE%s  %s", os::terminal::YELLOW, os::terminal::END, (dir << "game.ini")));
		GameIniData gi;
		gi.reset_default();
		gi.save(dir);
	}

	if (hui::config.has("EngineDir")) {
		Path engine_dir = hui::config.get_str("EngineDir");
		Path engine_api_dir = engine_dir << "api";
		auto list = os::fs::search(engine_api_dir, "*.kaba", "rf");
		for (auto &f: list) {
			auto dest = dir << "Scripts" << f;
			sync_files(engine_api_dir << f, dest);
		}
	}
}

void ModeAdministration::_new() {
	auto dlg = new NewProjectDialog(ed);
	hui::fly(dlg, [this, dlg] {
		if (dlg->ok) {
			create_project(dlg->directory, dlg->first_world);
			storage->set_root_directory(dlg->directory);
			data->reset();
		}
	});
}

bool ModeAdministration::open() {
	hui::file_dialog_dir(hui::CurWindow, _("Open project directory"), "", {}, [this] (const Path &path) {
		if (!path)
			return;
		if (!os::fs::exists(path << "game.ini")) {
			ed->error_box(_("game.ini not found"));
			//return false;
		}

		storage->set_root_directory(path);
		data->reset();
	});
	// TODO callback...
	return true;
}
