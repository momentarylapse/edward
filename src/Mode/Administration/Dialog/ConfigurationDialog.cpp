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

ConfigurationDialog::ConfigurationDialog(HuiWindow* _parent, bool _allow_parent, DataAdministration *_data, bool _exporting):
	HuiWindow(_exporting ? "ge_dialog" : "rc_dialog", _parent, _allow_parent)
{
	exporting = _exporting;
	data = _data;

	// dialog
	event("hui:close", this, &ConfigurationDialog::OnClose);
	event("cancel", this, &ConfigurationDialog::OnClose);
	event("ok", this, &ConfigurationDialog::OnOk);
	event("find_rootdir", this, &ConfigurationDialog::OnFindRootdir);
	event("find_default_world", this, &ConfigurationDialog::OnFindDefaultWorld);
	event("find_default_second_world", this, &ConfigurationDialog::OnFindDefaultSecondWorld);
	event("find_default_script", this, &ConfigurationDialog::OnFindDefaultScript);
	event("find_default_material", this, &ConfigurationDialog::OnFindDefaultMaterial);
	event("find_default_font", this, &ConfigurationDialog::OnFindDefaultFont);

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

	setString("rootdir",ed->RootDir);
}

void ConfigurationDialog::onUpdate(Observable *o)
{
}


void ConfigurationDialog::OnFindRootdir()
{
	if (HuiFileDialogDir(this,_("Arbeitsverzeichnis"),ed->RootDir))
		setString("rootdir", HuiFilename);
}

void ConfigurationDialog::OnFindDefaultWorld()
{
	if (ed->fileDialog(FDWorld, false, true))
		setString("default_world", ed->DialogFileNoEnding);
}

void ConfigurationDialog::OnFindDefaultSecondWorld()
{
	if (ed->fileDialog(FDWorld, false, true))
		setString("default_second_world", ed->DialogFileNoEnding);
}

void ConfigurationDialog::OnFindDefaultScript()
{
	if (ed->fileDialog(FDScript, false, true))
		setString("default_script", ed->DialogFile);
}

void ConfigurationDialog::OnFindDefaultMaterial()
{
	if (ed->fileDialog(FDMaterial, false, true))
		setString("default_material", ed->DialogFileNoEnding);
}

void ConfigurationDialog::OnFindDefaultFont()
{
	if (ed->fileDialog(FDFont, false, true))
		setString("default_font", ed->DialogFileNoEnding);
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
		bool rdc = (ed->RootDir != getString("rootdir"));
		if (rdc)
			ed->makeDirs(getString("rootdir"),true);
//		data->UnlinkGameIni();
		data->GameIni->DefWorld = getString("default_world");
		data->GameIni->SecondWorld = getString("default_second_world");
		data->GameIni->DefScript = getString("default_script");
		data->GameIni->DefMaterial = getString("default_material");
		data->GameIni->DefFont = getString("default_font");
//		data->LinkGameIni(data->GameIni);
		data->GameIni->Save(ed->RootDir);
		if (rdc)
			data->UpdateDatabase();
	}
	exporting = false;
	delete(this);
}

void ConfigurationDialog::OnClose()
{
	delete(this);
}
