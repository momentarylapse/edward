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

ConfigurationDialog::ConfigurationDialog(hui::Window* _parent, bool _allow_parent, DataAdministration *_data, bool _exporting):
	hui::Dialog(_exporting ? "ge_dialog" : "rc_dialog", 400, 300, _parent, _allow_parent),
	Observer("ConfigurationDialog")
{
	from_resource(_exporting ? "ge_dialog" : "rc_dialog");
	exporting = _exporting;
	data = _data;

	// dialog
	event("hui:close", [=]{ OnClose(); });
	event("cancel", [=]{ OnClose(); });
	event("ok", [=]{ OnOk(); });
	event("find_rootdir", [=]{ OnFindRootdir(); });
	event("find_default_world", [=]{ OnFindDefaultWorld(); });
	event("find_default_second_world", [=]{ OnFindDefaultSecondWorld(); });
	event("find_default_script", [=]{ OnFindDefaultScript(); });
	event("find_default_material", [=]{ OnFindDefaultMaterial(); });
	event("find_default_font", [=]{ OnFindDefaultFont(); });

	LoadData();
	subscribe(data);
}

ConfigurationDialog::~ConfigurationDialog()
{
	unsubscribe(data);
}

void ConfigurationDialog::LoadData()
{
	if (exporting){
		#ifdef HUI_OS_WINDOWS
			setInt("ged_system", 1);
		#endif
		set_int("export_type",0);
	}

	//GameIniDialog=GameIni;
//	data->LoadGameIni(ed->RootDir, &GameIniDialog);
	GameIniData GameIniDialog = *data->GameIni;
	set_string("default_world",GameIniDialog.DefWorld);
	set_string("default_second_world",GameIniDialog.SecondWorld);
	set_string("default_script",GameIniDialog.DefScript);
	set_string("default_material",GameIniDialog.DefMaterial);
	set_string("default_font",GameIniDialog.DefFont);

	set_string("rootdir",storage->root_dir);
}

void ConfigurationDialog::on_update(Observable *o, const string &message)
{
}


void ConfigurationDialog::OnFindRootdir()
{
	if (hui::FileDialogDir(this,_("Working directory"),storage->root_dir))
		set_string("rootdir", hui::Filename);
}

void ConfigurationDialog::OnFindDefaultWorld()
{
	if (storage->file_dialog(FD_WORLD, false, true))
		set_string("default_world", storage->dialog_file_no_ending);
}

void ConfigurationDialog::OnFindDefaultSecondWorld()
{
	if (storage->file_dialog(FD_WORLD, false, true))
		set_string("default_second_world", storage->dialog_file_no_ending);
}

void ConfigurationDialog::OnFindDefaultScript()
{
	if (storage->file_dialog(FD_SCRIPT, false, true))
		set_string("default_script", storage->dialog_file);
}

void ConfigurationDialog::OnFindDefaultMaterial()
{
	if (storage->file_dialog(FD_MATERIAL, false, true))
		set_string("default_material", storage->dialog_file_no_ending);
}

void ConfigurationDialog::OnFindDefaultFont()
{
	if (storage->file_dialog(FD_FONT, false, true))
		set_string("default_font", storage->dialog_file_no_ending);
}

void ConfigurationDialog::OnOk()
{
	if (exporting){
		//GameIniAlt=GameIni;
		GameIniData GameIniExport = *data->GameIni;
		string dir = get_string("rootdir");
		GameIniExport.DefWorld = get_string("default_world");
		GameIniExport.SecondWorld = get_string("default_second_world");
		GameIniExport.DefScript = get_string("default_script");
		GameIniExport.DefMaterial = get_string("default_material");
		GameIniExport.DefFont = get_string("default_font");
//		_exporting_type_ = getInt("export_type");
//		_exporting_system_ = getInt("ged_system");
		try{
			data->ExportGame(dir, GameIniExport);
		}catch(AdminGameExportException &e){
			ed->error_box(_("Error while exporting:") + e.message);
			return;
		}
	}else{
		// new RootDir?
		bool rdc = (storage->root_dir != get_string("rootdir"));
		if (rdc)
			storage->set_root_directory(get_string("rootdir"));
//		data->UnlinkGameIni();
		data->GameIni->DefWorld = get_string("default_world");
		data->GameIni->SecondWorld = get_string("default_second_world");
		data->GameIni->DefScript = get_string("default_script");
		data->GameIni->DefMaterial = get_string("default_material");
		data->GameIni->DefFont = get_string("default_font");
//		data->LinkGameIni(data->GameIni);
		data->GameIni->Save(storage->root_dir);
		if (rdc)
			data->UpdateDatabase();
	}
	exporting = false;
	destroy();
}

void ConfigurationDialog::OnClose()
{
	destroy();
}
