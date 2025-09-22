/*
 * ModeProject.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeProject.h"
/*#include "dialog/AdministrationDialog.h"
#include "dialog/ConfigurationDialog.h"
#include "dialog/NewProjectDialog.h"*/
#include <lib/xhui/config.h>
#include <lib/os/filesystem.h>
#include <lib/os/file.h>
#include <lib/os/terminal.h>
#include <lib/os/msg.h>
#include "data/DataProject.h"
#include "data/GameIniData.h"
#include <mode_world/data/DataWorld.h>
#include "../Session.h"
#include "../view/EdwardWindow.h"
#include "../storage/Storage.h"

ModeProject::ModeProject(DocumentSession *s): Mode(s)
	//Mode<ModeProject, DataAdministration>(s, "Administration", nullptr, new DataAdministration(s), nullptr, "menu_administration")
{
	dialog = nullptr;
}

ModeProject::~ModeProject() = default;

void ModeProject::on_enter() {
////	data->LoadDatabase();
////	data->UpdateDatabase();
////	dialog = new AdministrationDialog(session->win, true, data);
////	dialog->show();
}

void ModeProject::on_leave() {
	delete dialog;
}

void ModeProject::on_command(const string& id) {
	if (id == "export_game")
		export_game();
	if (id == "rudimentary_configuration")
		basic_settings();
}

void ModeProject::basic_settings() {
////	hui::fly(new ConfigurationDialog(hui::CurWindow, data, false));
}

void ModeProject::export_game() {
////	hui::fly(new ConfigurationDialog(hui::CurWindow, data, true));
	// data->ExportGame(...);
}


void ModeProject::create_project(const Path &dir, const string &first_world) {
	upgrade_project(dir);

	GameIniData gi;
	gi.load(dir);
	gi.set_str(GameIniData::ID_WORLD, first_world);
	gi.save(dir);

	Path world_file = dir | "Maps" | (first_world + ".world");
	msg_write(format("%sCREATE%s  %s", os::terminal::YELLOW, os::terminal::END, world_file));
	DataWorld w(doc);
	Storage s(session);
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

void ModeProject::upgrade_project(const Path &dir) {
	Storage s(session); // create CANONICAL_SUB_DIR[]

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

	if (xhui::config.has("EngineDir")) {
		Path engine_dir = xhui::config.get_str("EngineDir");
		Path engine_api_dir = engine_dir | "api";
		auto list = os::fs::search(engine_api_dir, "*", "rf");
		for (auto &f: list) {
			auto dest = dir | "Scripts" | f;
			bool required = true;//f.is_in("y");
			sync_files(engine_api_dir | f, dest, required);
		}
	} else {
		throw Exception("'EngineDir' is not set in config.txt");
	}
}

void ModeProject::_new() {
/*	auto dlg = new NewProjectDialog(session->win);
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
	});*/
}

bool ModeProject::open() {
/*	hui::file_dialog_dir(hui::CurWindow, _("Open project directory"), "", {}).then([this] (const Path &path) {
		if (!os::fs::exists(path | "game.ini")) {
			session->error(_("game.ini not found"));
			//return false;
		}

		data->session->storage->set_root_directory(path);
		data->reset();
	});
	// TODO callback...*/
	return true;
}
