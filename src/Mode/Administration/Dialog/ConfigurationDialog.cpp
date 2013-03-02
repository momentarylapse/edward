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

ConfigurationDialog::ConfigurationDialog(CHuiWindow* _parent, bool _allow_parent, DataAdministration *_data, bool _exporting):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	exporting = _exporting;
	data = _data;

	// dialog
	if (exporting)
		FromResource("ge_dialog");
	else
		FromResource("rc_dialog");
	EventM("hui:close", this, &ConfigurationDialog::OnClose);
	EventM("cancel", this, &ConfigurationDialog::OnClose);
	EventM("ok", this, &ConfigurationDialog::OnOk);
	EventM("find_rootdir", this, &ConfigurationDialog::OnFindRootdir);
	EventM("find_default_world", this, &ConfigurationDialog::OnFindDefaultWorld);
	EventM("find_default_second_world", this, &ConfigurationDialog::OnFindDefaultSecondWorld);
	EventM("find_default_script", this, &ConfigurationDialog::OnFindDefaultScript);
	EventM("find_default_material", this, &ConfigurationDialog::OnFindDefaultMaterial);
	EventM("find_default_font", this, &ConfigurationDialog::OnFindDefaultFont);

	LoadData();
	Subscribe(data);
}

ConfigurationDialog::~ConfigurationDialog()
{
	Unsubscribe(data);
}

void ConfigurationDialog::LoadData()
{
	if (exporting){
		#ifdef HUI_OS_WINDOWS
			SetInt("ged_system", 1);
		#endif
		SetInt("export_type",0);
	}

	//GameIniDialog=GameIni;
//	data->LoadGameIni(ed->RootDir, &GameIniDialog);
	GameIniData GameIniDialog = *data->GameIni;
	SetString("default_world",GameIniDialog.DefWorld);
	SetString("default_second_world",GameIniDialog.SecondWorld);
	SetString("default_script",GameIniDialog.DefScript);
	SetString("default_material",GameIniDialog.DefMaterial);
	SetString("default_font",GameIniDialog.DefFont);

	SetString("rootdir",ed->RootDir);
}

void ConfigurationDialog::OnUpdate(Observable *o)
{
}


void ConfigurationDialog::OnFindRootdir()
{
	if (HuiFileDialogDir(this,_("Arbeitsverzeichnis"),ed->RootDir))
		SetString("rootdir", HuiFilename);
}

void ConfigurationDialog::OnFindDefaultWorld()
{
	if (ed->FileDialog(FDWorld, false, true))
		SetString("default_world", ed->DialogFileNoEnding);
}

void ConfigurationDialog::OnFindDefaultSecondWorld()
{
	if (ed->FileDialog(FDWorld, false, true))
		SetString("default_second_world", ed->DialogFileNoEnding);
}

void ConfigurationDialog::OnFindDefaultScript()
{
	if (ed->FileDialog(FDScript, false, true))
		SetString("default_script", ed->DialogFile);
}

void ConfigurationDialog::OnFindDefaultMaterial()
{
	if (ed->FileDialog(FDMaterial, false, true))
		SetString("default_material", ed->DialogFileNoEnding);
}

void ConfigurationDialog::OnFindDefaultFont()
{
	if (ed->FileDialog(FDFont, false, true))
		SetString("default_font", ed->DialogFileNoEnding);
}

void ConfigurationDialog::OnOk()
{
	if (exporting){
		//GameIniAlt=GameIni;
		GameIniData GameIniExport = *data->GameIni;
		string dir = GetString("rootdir");
		GameIniExport.DefWorld = GetString("default_world");
		GameIniExport.SecondWorld = GetString("default_second_world");
		GameIniExport.DefScript = GetString("default_script");
		GameIniExport.DefMaterial = GetString("default_material");
		GameIniExport.DefFont = GetString("default_font");
//		_exporting_type_ = GetInt("export_type");
//		_exporting_system_ = GetInt("ged_system");
		try{
			data->ExportGame(dir, GameIniExport);
		}catch(AdminGameExportException &e){
			ed->ErrorBox(_("Fehler beim Exportieren: ") + e.message);
			return;
		}
	}else{
		// new RootDir?
		bool rdc = (ed->RootDir != GetString("rootdir"));
		if (rdc)
			ed->MakeDirs(GetString("rootdir"),true);
//		data->UnlinkGameIni();
		data->GameIni->DefWorld = GetString("default_world");
		data->GameIni->SecondWorld = GetString("default_second_world");
		data->GameIni->DefScript = GetString("default_script");
		data->GameIni->DefMaterial = GetString("default_material");
		data->GameIni->DefFont = GetString("default_font");
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
