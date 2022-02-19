/*
 * ModeAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ModeAdministration.h"
#include "../../Data/Administration/DataAdministration.h"
#include "../../Data/Administration/GameIniData.h"
#include "Dialog/AdministrationDialog.h"
#include "Dialog/ConfigurationDialog.h"
#include "Dialog/NewProjectDialog.h"
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
		ExportGame();
	if (id == "rudimentary_configuration")
		BasicSettings();
}

void ModeAdministration::BasicSettings() {
	hui::fly(new ConfigurationDialog(hui::CurWindow, data, false));
}

void ModeAdministration::ExportGame() {
	hui::fly(new ConfigurationDialog(hui::CurWindow, data, true));
	// data->ExportGame(...);
}


void ModeAdministration::create_project_dir(const Path &dir) {
	for (int k=0; k<NUM_FDS; k++)
		dir_create(dir << Storage::CANONICAL_SUB_DIR[k]);

	if (!file_exists(dir << "game.ini")) {
		GameIniData gi;
		gi.save(dir);
	}
}

void ModeAdministration::_new() {
	auto dlg = new NewProjectDialog(ed);
	hui::fly(dlg, [this, dlg] {
		if (dlg->ok) {
			create_project_dir(dlg->directory);
			storage->set_root_directory(dlg->directory);
			data->reset();
		}
	});
}

bool ModeAdministration::open() {
	hui::file_dialog_dir(hui::CurWindow, _("Open project directory"), "", {}, [this] (const Path &path) {
		if (!path)
			return;
		if (!file_exists(path << "game.ini")) {
			ed->error_box(_("game.ini not found"));
			//return false;
		}

		storage->set_root_directory(path);
		data->reset();
	});
	// TODO callback...
	return true;
}
