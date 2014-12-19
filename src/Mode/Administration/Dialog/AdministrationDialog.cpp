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
#include "../../../meta.h"
#include "../../../x/model_manager.h"
#include "../../../x/font.h"
#include "../../../Mode/Welcome/ModeWelcome.h"
#include "../../../Mode/Model/ModeModel.h"
#include "../../../Mode/Material/ModeMaterial.h"
#include "../../../Mode/World/ModeWorld.h"
#include "../../../Mode/Font/ModeFont.h"
#include "../ModeAdministration.h"
#include <assert.h>

AdministrationDialog::AdministrationDialog(HuiWindow* _parent, bool _allow_parent, DataAdministration *_data):
	HuiWindow("ad_dialog", _parent, _allow_parent),
	Observer("AdministrationDialog")
{
	data = _data;
	file_list.resize(6);

	// dialog
	event("hui:close", this, &AdministrationDialog::OnClose);
	event("exit", this, &AdministrationDialog::OnExit);
	event("ad_edit", this, &AdministrationDialog::OnEdit);
	event("rename", this, &AdministrationDialog::OnRename);
	event("delete", this, &AdministrationDialog::OnDelete);
	event("file_list_cur", this, &AdministrationDialog::OnFileList);
	event("file_list_all", this, &AdministrationDialog::OnFileList);
	event("file_list_detail_source", this, &AdministrationDialog::OnFileList);
	event("file_list_detail_dest", this, &AdministrationDialog::OnFileList);
	event("file_list_super", this, &AdministrationDialog::OnFileList);
	event("file_list_missing", this, &AdministrationDialog::OnFileList);
	event("ad_rudimentary_configuration", this, &AdministrationDialog::OnRudimentaryConfiguration);
	event("ad_export_game", this, &AdministrationDialog::OnExportGame);

	LoadData();
	subscribe(data);
}

AdministrationDialog::~AdministrationDialog()
{
	unsubscribe(data);
}

void AdministrationDialog::LoadData()
{
	FillAdminList(0, "file_list_cur");
	FillAdminList(1, "file_list_all");
	FillAdminList(4, "file_list_super");
	FillAdminList(5, "file_list_missing");
}

void AdministrationDialog::onUpdate(Observable* o, const string &message)
{
	LoadData();
}



static string FD2Str(int k)
{
	if (k==-1)				return _("[Engine]");
	if (k==FD_MODEL)			return _("Modell");
	if (k==FD_TEXTURE)		return _("Textur");
	if (k==FD_SOUND)			return _("Sound");
	if (k==FD_MATERIAL)		return _("Material");
	if (k==FD_TERRAIN)		return _("Terrain");
	if (k==FD_WORLD)			return _("Welt");
	if (k==FD_SHADERFILE)	return _("Shader");
	if (k==FD_FONT)			return _("Font");
	if (k==FD_SCRIPT)		return _("Script");
	if (k==FD_CAMERAFLIGHT)	return _("Kamera");
	if (k==FD_FILE)			return _("Datei");
	return "???";
}

void AdministrationDialog::FillAdminList(int view, const string &lid)
{
	msg_db_f("FillAdminList",1);

	reset(lid);
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
		addString(lid, format("%s::%s::%d::%d::%s",
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
}

void AdministrationDialog::ShowDetail(int n, const string &lid)
{
	msg_db_f("ShowDetail", 1);
	AdminFileList *l = get_list(lid);
	assert(l);
	assert(n >= 0);
	assert(n < l->num);
	SelectedAdminFile = (*l)[n];
	setString("file_details", SelectedAdminFile->Name);
	FillAdminList(2, "file_list_detail_source");
	FillAdminList(3, "file_list_detail_dest");
	setInt("ad_tab_control", 2);
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
	Array<int> index = getSelection(lid);
	AdminFileList *l = get_list(lid);
	assert(l);
	foreach(int i, index)
		r.add((*l)[i]);
	return r;
}

Array<AdminFile*> AdministrationDialog::GetSelectedFiles()
{
	int page = getInt("ad_tab_control");
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
	ed->errorBox(_("Es muss genau eine Datei markiert sein!"));
	return NULL;
}

void AdministrationDialog::OnClose()
{
	ed->setMode(mode_welcome);
}

void AdministrationDialog::OnExit()
{	ed->setMode(mode_welcome);	}

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
				HuiOpenDocument(ed->getRootDir(a->Kind) + a->Name);
			else if (a->Name == "game.ini")
				mode_administration->BasicSettings();
			break;
		case FD_MODEL:
			if (mode_model->data->load(ObjectDir + a->Name, true))
				ed->setMode(mode_model);
			break;
		case FD_MATERIAL:
			if (mode_material->data->load(MaterialDir + a->Name, true))
				ed->setMode(mode_material);
			break;
		case FD_FONT:
			if (mode_font->data->load(Gui::FontDir + a->Name, true))
				ed->setMode(mode_font);
			break;
		case FD_WORLD:
			if (mode_world->data->load(MapDir + a->Name, true))
				ed->setMode(mode_world);
			break;
		case FD_TERRAIN:
			mode_world->data->reset();
			if (mode_world->data->AddTerrain(a->Name.substr(0, -5), v_0)){
				ed->setMode(mode_world);
			}
			break;
		case FD_CAMERAFLIGHT:
			/*mode_world->data->Reset();
			strcpy(mworld->CamScriptFile,a->Name);
			if (mworld->LoadCameraScript()){
				SetMode(ModeWorld);
				mworld->OptimizeView();
			}*/
			break;
		case FD_TEXTURE:
		case FD_SOUND:
		case FD_SHADERFILE:
		case FD_SCRIPT:
		case FD_FILE:
			HuiOpenDocument(ed->getRootDir(a->Kind) + a->Name);
			break;
	}
}

void AdministrationDialog::OnFileList()
{
	string id = HuiGetEvent()->id;
	int n = getInt(id);
	ShowDetail(n, id);
}

void AdministrationDialog::OnRudimentaryConfiguration()
{
	mode_administration->BasicSettings();
}

