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

ModeAdministration *mode_administration;

ModeAdministration::ModeAdministration():
	Mode<DataAdministration>("Administration", NULL, new DataAdministration, NULL, "menu_administration")
{
	dialog = NULL;
}

ModeAdministration::~ModeAdministration()
{
}

void ModeAdministration::onStart()
{
	data->LoadDatabase();
	data->UpdateDatabase();
	dialog = new AdministrationDialog(ed, true, data);
	dialog->show();
}

void ModeAdministration::onEnd()
{
	delete(dialog);
}

void ModeAdministration::onCommand(const string& id)
{
	if (id == "export_game")
		ExportGame();
	if (id == "rudimentary_configuration")
		BasicSettings();
}

void ModeAdministration::onUpdate(Observable* o, const string &message)
{
}

void ModeAdministration::BasicSettings()
{
	ConfigurationDialog *dlg = new ConfigurationDialog(hui::CurWindow, false, data, false);
	dlg->run();
	delete dlg;
}

void ModeAdministration::ExportGame()
{
	ConfigurationDialog *dlg = new ConfigurationDialog(hui::CurWindow, false, data, true);
	dlg->run();
	delete dlg;

	// data->ExportGame(...);
}


void ModeAdministration::create_project_dir(const string &dir)
{
	dir_create(dir + "Fonts");
	dir_create(dir + "Materials");
	dir_create(dir + "Maps");
	dir_create(dir + "Objects");
	dir_create(dir + "Scripts");
	dir_create(dir + "Sounds");
	dir_create(dir + "Textures");

	if (!file_test_existence(dir + "game.ini")){
		GameIniData gi;
		gi.Save(dir);
	}

	if (!file_test_existence(dir + "config.txt")){
		File *f = FileCreateText(dir + "config.txt");
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

void ModeAdministration::_new()
{
	if (!hui::FileDialogDir(hui::CurWindow, _("Verzeichnis für neues Projekt wählen"), ""))
		return;

	create_project_dir(hui::Filename);
	ed->setRootDirectory(hui::Filename);
	data->reset();
}

bool ModeAdministration::open()
{
	if (!hui::FileDialogDir(hui::CurWindow, _("Projekt-Verzeichnis öffnen"), ""))
		return false;

	if (!file_test_existence(hui::Filename + "game.ini")){
		ed->errorBox(_("game.ini nicht gefunden"));
		return false;
	}

	ed->setRootDirectory(hui::Filename);
	data->reset();
	return true;
}
