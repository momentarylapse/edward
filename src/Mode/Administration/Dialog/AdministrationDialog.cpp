/*
 * AdministrationDialog.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "AdministrationDialog.h"
#include "../../../Data/Administration/DataAdministration.h"
#include "../../../Data/Administration/AdminFile.h"
#include "../../../Data/Administration/AdminFileList.h"
#include "../../../Edward.h"
#include "../../../Mode/Welcome/ModeWelcome.h"
#include "../../../Mode/Model/ModeModel.h"
#include "../../../Mode/Material/ModeMaterial.h"
#include "../../../Mode/World/ModeWorld.h"
#include "../../../Mode/Font/ModeFont.h"
#include "../ModeAdministration.h"
#include <assert.h>

AdministrationDialog::AdministrationDialog(CHuiWindow* _parent, bool _allow_parent, DataAdministration *_data):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	data = _data;
	file_list.resize(6);

	// dialog
	FromResource("ad_dialog");
	EventM("hui:close", this, &AdministrationDialog::OnClose);
	EventM("exit", this, &AdministrationDialog::OnExit);
	EventM("ad_edit", this, &AdministrationDialog::OnEdit);
	EventM("rename", this, &AdministrationDialog::OnRename);
	EventM("delete", this, &AdministrationDialog::OnDelete);
	EventM("file_list_cur", this, &AdministrationDialog::OnFileList);
	EventM("file_list_all", this, &AdministrationDialog::OnFileList);
	EventM("file_list_detail_source", this, &AdministrationDialog::OnFileList);
	EventM("file_list_detail_dest", this, &AdministrationDialog::OnFileList);
	EventM("file_list_super", this, &AdministrationDialog::OnFileList);
	EventM("file_list_missing", this, &AdministrationDialog::OnFileList);
	EventM("ad_rudimentary_configuration", this, &AdministrationDialog::OnRudimentaryConfiguration);
	EventM("ad_export_game", this, &AdministrationDialog::OnExportGame);

	LoadData();
	Subscribe(data);
}

AdministrationDialog::~AdministrationDialog()
{
	Unsubscribe(data);
}

void AdministrationDialog::LoadData()
{
	FillAdminList(0, "file_list_cur");
	FillAdminList(1, "file_list_all");
	FillAdminList(4, "file_list_super");
	FillAdminList(5, "file_list_missing");
}

void AdministrationDialog::OnUpdate(Observable* o)
{
	LoadData();
}



static string FD2Str(int k)
{
	if (k==-1)				return _("[Engine]");
	if (k==FDModel)			return _("Modell");
	if (k==FDTexture)		return _("Textur");
	if (k==FDSound)			return _("Sound");
	if (k==FDMaterial)		return _("Material");
	if (k==FDTerrain)		return _("Terrain");
	if (k==FDWorld)			return _("Welt");
	if (k==FDShaderFile)	return _("Shader");
	if (k==FDFont)			return _("Font");
	if (k==FDScript)		return _("Script");
	if (k==FDCameraFlight)	return _("Kamera");
	if (k==FDFile)			return _("Datei");
	return "???";
}

void AdministrationDialog::FillAdminList(int view, const string &lid)
{
	msg_db_r("FillAdminList",1);

	Reset(lid);
	string sep = HuiComboBoxSeparator;
	HuiComboBoxSeparator = "::";

	// currently viewed list
	AdminFileList *l = get_list(lid);
	assert(l);

	l->clear();
	if (view == 0){ // current game (in game.ini)
		l->add_recursive((*data->file_list)[0]);
	}else if (view == 1){ // all files
		*l = *data->file_list;
	}else if (view == 2){ // selected file
		foreach(AdminFile *a, SelectedAdminFile->Parent)
			l->add(a);
	}else if (view == 3){
		foreach(AdminFile *a, SelectedAdminFile->Child)
			l->add(a);
	}else if (view == 4){ // unnessecary
		foreach(AdminFile *a, *data->file_list)
			if ((a->Kind >= 0) && (a->Parent.num == 0))
				l->add(a);
	}else if (view == 5){ // missing
		foreach(AdminFile *a, *data->file_list)
			if (a->Missing)
				l->add(a);
	}


	l->sort();

	// place them into the list
	int k=-2;
	foreach(AdminFile *a, *l){
		AddString(lid, format("%s::%s::%d::%d::%s",
			(k != a->Kind) ? FD2Str(a->Kind).c_str() : "",
			a->Name.c_str(),
			a->Parent.num, a->Child.num,
			a->Missing ? "1" : "0"));
		k = a->Kind;
	}
#ifdef NIX_OS_WINDOWS
	AddString(lid, ""); // windows ... -> doesn't rescale the list's columns automatically  m(-_-)m
#endif
	HuiComboBoxSeparator = sep;
	msg_db_l(1);
}

void AdministrationDialog::ShowDetail(int n, const string &lid)
{
	msg_db_r("ShowDetail", 1);
	AdminFileList *l = get_list(lid);
	assert(l);
	assert(n >= 0);
	assert(n < l->num);
	SelectedAdminFile = (*l)[n];
	SetString("file_details", SelectedAdminFile->Name);
	FillAdminList(2, "file_list_detail_source");
	FillAdminList(3, "file_list_detail_dest");
	SetInt("ad_tab_control", 2);
	msg_db_l(1);
}


void AdministrationDialog::OnExportGame()
{
	mode_administration->ExportGame();
}

AdminFileList *AdministrationDialog::get_list(const string &lid)
{
	if (lid == "file_list_cur")
		return &file_list[0];
	if (lid == "file_list_all")
		return &file_list[1];
	if (lid == "file_list_detail_source")
		return &file_list[2];
	if (lid == "file_list_detail_dest")
		return &file_list[3];
	if (lid == "file_list_super")
		return &file_list[4];
	if (lid == "file_list_missing")
		return &file_list[5];
	return NULL;
}

string get_first_list_id_by_tab_page(int page)
{
	if (page == 0)
		return "file_list_cur";
	if (page == 1)
		return "file_list_all";
	if (page == 2)
		return "file_list_detail_source";
	if (page == 3)
		return "file_list_super";
	if (page == 4)
		return "file_list_missing";
	return "";
}

Array<AdminFile*> AdministrationDialog::GetSelectedFilesFromList(const string& lid)
{
	Array<AdminFile*> r;
	Array<int> index = GetMultiSelection(lid);
	AdminFileList *l = get_list(lid);
	assert(l);
	foreach(int i, index)
		r.add((*l)[i]);
	return r;
}

Array<AdminFile*> AdministrationDialog::GetSelectedFiles()
{
	int page = GetInt("ad_tab_control");
	Array<AdminFile*> r = GetSelectedFilesFromList(get_first_list_id_by_tab_page(page));
	if (page == 2)
		r.append(GetSelectedFilesFromList("file_list_detail_dest"));
	return r;
}

AdminFile* AdministrationDialog::GetSingleSelectedFile()
{
	Array<AdminFile*> l = GetSelectedFiles();
	if (l.num == 1)
		return l[0];
	ed->ErrorBox(_("Es muss genau eine Datei markiert sein!"));
	return NULL;
}

void AdministrationDialog::OnClose()
{
	ed->SetMode(mode_welcome);
}

void AdministrationDialog::OnExit()
{	ed->SetMode(mode_welcome);	}

void AdministrationDialog::OnRename()
{}//{	data->Rename();	}

void AdministrationDialog::OnDelete()
{}//{	madmin->Delete();	}

void AdministrationDialog::OnEdit()
{
	AdminFile *a = GetSingleSelectedFile();
	if (!a)
		return;
	switch (a->Kind){
		case -1:
			if (a->Name == "config.txt")
				HuiOpenDocument(ed->GetRootDir(a->Kind) + a->Name);
			else if (a->Name == "game.ini")
				mode_administration->BasicSettings();
			break;
		case FDModel:
			if (mode_model->data->Load(ObjectDir + a->Name, true))
				ed->SetMode(mode_model);
			break;
		case FDMaterial:
			if (mode_material->data->Load(MaterialDir + a->Name, true))
				ed->SetMode(mode_material);
			break;
		case FDFont:
			if (mode_font->data->Load(MaterialDir + a->Name, true))
				ed->SetMode(mode_font);
			break;
		case FDWorld:
			if (mode_world->data->Load(MapDir + a->Name, true))
				ed->SetMode(mode_world);
			break;
		case FDTerrain:
			mode_world->data->Reset();
			if (mode_world->data->AddTerrain(a->Name.substr(0, -5), v_0)){
				ed->SetMode(mode_world);
			}
			break;
		case FDCameraFlight:
			/*mode_world->data->Reset();
			strcpy(mworld->CamScriptFile,a->Name);
			if (mworld->LoadCameraScript()){
				SetMode(ModeWorld);
				mworld->OptimizeView();
			}*/
			break;
		case FDTexture:
		case FDSound:
		case FDShaderFile:
		case FDScript:
		case FDFile:
			HuiOpenDocument(ed->GetRootDir(a->Kind) + a->Name);
			break;
	}
}

void AdministrationDialog::OnFileList()
{
	string id = HuiGetEvent()->id;
	int n = GetInt(id);
	ShowDetail(n, id);
}

void AdministrationDialog::OnRudimentaryConfiguration()
{
	mode_administration->BasicSettings();
}

