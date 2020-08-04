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

	if (!file_exists(dir << "config.txt")) {
		File *f = FileCreateText(dir << "config.txt");
		f->write_comment("// ScreenWidth");
		f->write_int(800);
		f->write_comment("// ScreenHeight");
		f->write_int(600);
		f->write_comment("// ScreenDepth");
		f->write_int(32);
		f->write_comment("// Graphics-API");
		f->write_str("OpenGL");
		f->write_comment("// Fullscreen");
		f->write_int(0);
		f->write_comment("// Debug");
		f->write_int(1);
		f->write_comment("// ShadowLevel");
		f->write_int(0);
		f->write_comment("// MirrorLevelMax");
		f->write_int(1);
		f->write_comment("// DetailLevel");
		f->write_int(100);
		f->write_comment("// FpsMax");
		f->write_float(60.000);
		f->write_comment("// FpsMin");
		f->write_float(10.000);
		f->write_comment("// MaxVideoTextureSize");
		f->write_int(256);
		f->write_comment("// Multisampling");
		f->write_int(0);
		f->write_comment("// Network");
		f->write_int(0);
		f->write_comment("// HostNames");
		f->write_str("");
		f->write_comment("// Console");
		f->write_int(0);
		f->write_str("#");
		delete f;
	}
}

void ModeAdministration::_new() {
	if (!hui::FileDialogDir(hui::CurWindow, _("Choose a directory for the new project"), ""))
		return;

	create_project_dir(hui::Filename);
	storage->set_root_directory(hui::Filename);
	data->reset();
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
