/*
 * ModeAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeAdministration.h"
#include "dialog/AdministrationDialog.h"
#include "dialog/ConfigurationDialog.h"
#include "dialog/NewProjectDialog.h"
#include "../../lib/os/filesystem.h"
#include "../../lib/os/file.h"
#include "../../lib/os/terminal.h"
#include "../../data/administration/DataAdministration.h"
#include "../../data/administration/GameIniData.h"
#include "../../data/world/DataWorld.h"
#include "../../Session.h"
#include "../../EdwardWindow.h"
#include "../../storage/Storage.h"

ModeAdministration::ModeAdministration(Session *s):
	Mode<ModeAdministration, DataAdministration>(s, "Administration", nullptr, new DataAdministration(s), nullptr, "menu_administration")
{
	dialog = nullptr;
}

ModeAdministration::~ModeAdministration() {
}

void ModeAdministration::on_start() {
	data->LoadDatabase();
	data->UpdateDatabase();
	dialog = new AdministrationDialog(session->win, true, data);
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

	Path world_file = dir | "Maps" | (first_world + ".world");
	msg_write(format("%sCREATE%s  %s", os::terminal::YELLOW, os::terminal::END, world_file));
	DataWorld w(data->session);
	Storage s(data->session);
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

static void sync_files(const Path &source, const Path &dest, bool required) {
	create_directory_recursive(dest.parent());
	if (os::fs::exists(dest)) {
		if (os::fs::read_binary(dest) == os::fs::read_binary(source)) {
			msg_write(format("%sOK%s  %s", os::terminal::GREEN, os::terminal::END, dest));
			return;
		}
	} else if (!required) {
		return;
	}
	msg_write(format("%sCOPY%s  %s", os::terminal::YELLOW, os::terminal::END, dest));
	os::fs::copy(source, dest);
}

void ModeAdministration::upgrade_project(const Path &dir) {
	Storage s(data->session); // create CANONICAL_SUB_DIR[]

	for (int k=0; k<NUM_FDS; k++)
		create_directory_recursive(dir | Storage::CANONICAL_SUB_DIR[k]);

	if (os::fs::exists(dir | "game.ini")) {
		msg_write(format("%sOK%s  %s", os::terminal::GREEN, os::terminal::END, (dir | "game.ini")));
	} else {
		msg_write(format("%sCREATE%s  %s", os::terminal::YELLOW, os::terminal::END, (dir | "game.ini")));
		GameIniData gi;
		gi.reset_default();
		gi.save(dir);
	}

	if (hui::config.has("EngineDir")) {
		Path engine_dir = hui::config.get_str("EngineDir");
		Path engine_api_dir = engine_dir | "api";
		auto list = os::fs::search(engine_api_dir, "*.kaba", "rf");
		for (auto &f: list) {
			auto dest = dir | "Scripts" | f;
			bool required = true;//f.is_in("y");
			sync_files(engine_api_dir | f, dest, required);
		}
	} else {
		throw Exception("'EngineDir' is not set in config.txt");
	}
}

void ModeAdministration::_new() {
	auto dlg = new NewProjectDialog(session->win);
	hui::fly(dlg).then([this, dlg] {
		if (dlg->ok) {
			try {
				create_project(dlg->directory, dlg->first_world);
				data->session->storage->set_root_directory(dlg->directory);
				data->reset();
			} catch (Exception &e) {
				session->error(e.message());
			}
		}
	});
}

bool ModeAdministration::open() {
	hui::file_dialog_dir(hui::CurWindow, _("Open project directory"), "", {}).then([this] (const Path &path) {
		if (!os::fs::exists(path | "game.ini")) {
			session->error(_("game.ini not found"));
			//return false;
		}

		data->session->storage->set_root_directory(path);
		data->reset();
	});
	// TODO callback...
	return true;
}
