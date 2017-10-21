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

ConfigurationDialog::ConfigurationDialog(hui::Window* _parent, bool _allow_parent, DataAdministration *_data, bool _exporting):
	hui::Dialog(_exporting ? "ge_dialog" : "rc_dialog", 400, 300, _parent, _allow_parent),
	Observer("ConfigurationDialog")
{
	fromResource(_exporting ? "ge_dialog" : "rc_dialog");
	exporting = _exporting;
	data = _data;

	// dialog
	event("hui:close", std::bind(&ConfigurationDialog::OnClose, this));
	event("cancel", std::bind(&ConfigurationDialog::OnClose, this));
	event("ok", std::bind(&ConfigurationDialog::OnOk, this));
	event("find_rootdir", std::bind(&ConfigurationDialog::OnFindRootdir, this));
	event("find_default_world", std::bind(&ConfigurationDialog::OnFindDefaultWorld, this));
	event("find_default_second_world", std::bind(&ConfigurationDialog::OnFindDefaultSecondWorld, this));
	event("find_default_script", std::bind(&ConfigurationDialog::OnFindDefaultScript, this));
	event("find_default_material", std::bind(&ConfigurationDialog::OnFindDefaultMaterial, this));
	event("find_default_font", std::bind(&ConfigurationDialog::OnFindDefaultFont, this));

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
		setInt("export_type",0);
	}

	//GameIniDialog=GameIni;
//	data->LoadGameIni(ed->RootDir, &GameIniDialog);
	GameIniData GameIniDialog = *data->GameIni;
	setString("default_world",GameIniDialog.DefWorld);
	setString("default_second_world",GameIniDialog.SecondWorld);
	setString("default_script",GameIniDialog.DefScript);
	setString("default_material",GameIniDialog.DefMaterial);
	setString("default_font",GameIniDialog.DefFont);

	setString("rootdir",ed->root_dir);
}

void ConfigurationDialog::onUpdate(Observable *o, const string &message)
{
}


void ConfigurationDialog::OnFindRootdir()
{
	if (hui::FileDialogDir(this,_("Arbeitsverzeichnis"),ed->root_dir))
		setString("rootdir", hui::Filename);
}

void ConfigurationDialog::OnFindDefaultWorld()
{
	if (ed->fileDialog(FD_WORLD, false, true))
		setString("default_world", ed->dialog_file_no_ending);
}

void ConfigurationDialog::OnFindDefaultSecondWorld()
{
	if (ed->fileDialog(FD_WORLD, false, true))
		setString("default_second_world", ed->dialog_file_no_ending);
}

void ConfigurationDialog::OnFindDefaultScript()
{
	if (ed->fileDialog(FD_SCRIPT, false, true))
		setString("default_script", ed->dialog_file);
}

void ConfigurationDialog::OnFindDefaultMaterial()
{
	if (ed->fileDialog(FD_MATERIAL, false, true))
		setString("default_material", ed->dialog_file_no_ending);
}

void ConfigurationDialog::OnFindDefaultFont()
{
	if (ed->fileDialog(FD_FONT, false, true))
		setString("default_font", ed->dialog_file_no_ending);
}

void ConfigurationDialog::OnOk()
{
	if (exporting){
		//GameIniAlt=GameIni;
		GameIniData GameIniExport = *data->GameIni;
		string dir = getString("rootdir");
		GameIniExport.DefWorld = getString("default_world");
		GameIniExport.SecondWorld = getString("default_second_world");
		GameIniExport.DefScript = getString("default_script");
		GameIniExport.DefMaterial = getString("default_material");
		GameIniExport.DefFont = getString("default_font");
//		_exporting_type_ = getInt("export_type");
//		_exporting_system_ = getInt("ged_system");
		try{
			data->ExportGame(dir, GameIniExport);
		}catch(AdminGameExportException &e){
			ed->errorBox(_("Fehler beim Exportieren: ") + e.message);
			return;
		}
	}else{
		// new RootDir?
		bool rdc = (ed->root_dir != getString("rootdir"));
		if (rdc)
			ed->makeDirs(getString("rootdir"),true);
//		data->UnlinkGameIni();
		data->GameIni->DefWorld = getString("default_world");
		data->GameIni->SecondWorld = getString("default_second_world");
		data->GameIni->DefScript = getString("default_script");
		data->GameIni->DefMaterial = getString("default_material");
		data->GameIni->DefFont = getString("default_font");
//		data->LinkGameIni(data->GameIni);
		data->GameIni->Save(ed->root_dir);
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
