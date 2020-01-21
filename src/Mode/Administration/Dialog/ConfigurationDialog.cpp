/*
 * ConfigurationDialog.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "ConfigurationDialog.h"
#include "../../../Data/Administration/DataAdministration.h"
#include "../../../Data/Administration/GameIniData.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"

ConfigurationDialog::ConfigurationDialog(hui::Window* _parent, DataAdministration *_data, bool _exporting):
	hui::Dialog(_exporting ? "ge_dialog" : "game-config-dialog", 400, 300, _parent, false)
{
	from_resource(_exporting ? "ge_dialog" : "game-config-dialog");
	exporting = _exporting;
	data = _data;

	// dialog
	event("hui:close", [=]{ on_close(); });
	event("cancel", [=]{ on_close(); });
	event("ok", [=]{ on_ok(); });
	//event("find-rootdir", [=]{ OnFindRootdir(); });
	event("find-world", [=]{ on_find_world(); });
	event("find-second-world", [=]{ on_find_second_world(); });
	event("find-script", [=]{ on_find_script(); });
	event("find-material", [=]{ on_find_material(); });
	event("find-font", [=]{ on_find_font(); });

	load_data();
}

void ConfigurationDialog::load_data()
{
	if (exporting){
		#ifdef HUI_OS_WINDOWS
			setInt("ged_system", 1);
		#endif
		set_int("export_type",0);
	}

	//GameIniDialog=GameIni;
//	data->LoadGameIni(ed->RootDir, &GameIniDialog);
	//GameIniData game = *data->GameIni;
	GameIniData game;
	game.Load(storage->root_dir);
	set_string("world",game.DefWorld);
	set_string("second-world",game.SecondWorld);
	set_string("script",game.DefScript);
	set_string("material",game.DefMaterial);
	set_string("font",game.DefFont);

	set_string("root-directory",storage->root_dir);
}


void ConfigurationDialog::on_find_root_dir() {
	if (hui::FileDialogDir(this,_("Working directory"),storage->root_dir))
		set_string("root-directory", hui::Filename);
}

void ConfigurationDialog::on_find_world() {
	if (storage->file_dialog(FD_WORLD, false, true))
		set_string("world", storage->dialog_file_no_ending);
}

void ConfigurationDialog::on_find_second_world() {
	if (storage->file_dialog(FD_WORLD, false, true))
		set_string("second-world", storage->dialog_file_no_ending);
}

void ConfigurationDialog::on_find_script() {
	if (storage->file_dialog(FD_SCRIPT, false, true))
		set_string("script", storage->dialog_file);
}

void ConfigurationDialog::on_find_material() {
	if (storage->file_dialog(FD_MATERIAL, false, true))
		set_string("material", storage->dialog_file_no_ending);
}

void ConfigurationDialog::on_find_font() {
	if (storage->file_dialog(FD_FONT, false, true))
		set_string("font", storage->dialog_file_no_ending);
}

void ConfigurationDialog::on_ok() {
	if (exporting) {
		//GameIniAlt=GameIni;
		GameIniData GameIniExport = *data->GameIni;
		string dir = get_string("root-directory");
		GameIniExport.DefWorld = get_string("world");
		GameIniExport.SecondWorld = get_string("second-world");
		GameIniExport.DefScript = get_string("script");
		GameIniExport.DefMaterial = get_string("material");
		GameIniExport.DefFont = get_string("font");
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
		data->GameIni->DefWorld = get_string("world");
		data->GameIni->SecondWorld = get_string("second-world");
		data->GameIni->DefScript = get_string("script");
		data->GameIni->DefMaterial = get_string("material");
		data->GameIni->DefFont = get_string("font");
//		data->LinkGameIni(data->GameIni);
		data->GameIni->Save(storage->root_dir);
		if (rdc)
			data->UpdateDatabase();
	}
	exporting = false;
	destroy();
}

void ConfigurationDialog::on_close() {
	destroy();
}
