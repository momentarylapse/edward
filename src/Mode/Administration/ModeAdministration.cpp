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

ModeAdministration *mode_administration;

ModeAdministration::ModeAdministration():
	Mode<DataAdministration>("Administration", NULL, new DataAdministration, NULL, "menu_administration")
{
	dialog = NULL;
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
	auto dlg = new ConfigurationDialog(hui::CurWindow, data, false);
	dlg->run();
	delete dlg;
}

void ModeAdministration::ExportGame() {
	auto dlg = new ConfigurationDialog(hui::CurWindow, data, true);
	dlg->run();
	delete dlg;

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
	dlg->run();
	if (dlg->ok) {

		create_project_dir(dlg->directory);
		storage->set_root_directory(dlg->directory);
		data->reset();
	}
	delete dlg;
}

bool ModeAdministration::open() {
	if (!hui::FileDialogDir(hui::CurWindow, _("Open project directory"), ""))
		return false;

	if (!file_exists(hui::Filename << "game.ini")) {
		ed->error_box(_("game.ini not found"));
		return false;
	}

	storage->set_root_directory(hui::Filename);
	data->reset();
	return true;
}
