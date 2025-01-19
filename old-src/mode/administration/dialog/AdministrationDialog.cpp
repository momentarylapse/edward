/*
 * AdministrationDialog.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "AdministrationDialog.h"
#include "../../../data/administration/DataAdministration.h"
#include "../../../data/administration/AdminFile.h"
#include "../../../data/administration/AdminFileList.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include <y/EngineData.h>
#include <y/gui/Font.h>
#include "../../../mode/model/ModeModel.h"
#include "../../../mode/material/ModeMaterial.h"
#include "../../../mode/world/ModeWorld.h"
#include "../../../mode/font/ModeFont.h"
#include "../ModeAdministration.h"
#include <assert.h>

#include <y/world/ModelManager.h>

AdministrationDialog::AdministrationDialog(hui::Window* _parent, bool _allow_parent, DataAdministration *_data):
	obs::Node<hui::Dialog>("ad_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("ad_dialog");
	data = _data;
	file_list.resize(6);

	// dialog
	event("hui:close", [this] { OnClose(); });
	event("exit", [this] { OnExit(); });
	event("ad_edit", [this] { OnEdit(); });
	event("rename", [this] { OnRename(); });
	event("delete", [this] { OnDelete(); });
	event("file_list_cur", [this] { OnFileList(); });
	event("file_list_all", [this] { OnFileList(); });
	event("file_list_detail_source", [this] { OnFileList(); });
	event("file_list_detail_dest", [this] { OnFileList(); });
	event("file_list_super", [this] { OnFileList(); });
	event("file_list_missing", [this] { OnFileList(); });
	event("ad_rudimentary_configuration", [this] { OnRudimentaryConfiguration(); });
	event("ad_export_game", [this] { OnExportGame(); });

	LoadData();
	data->out_changed >> create_sink([this] { LoadData(); });
}

AdministrationDialog::~AdministrationDialog() {
	data->unsubscribe(this);
}

void AdministrationDialog::LoadData() {
	FillAdminList(0, "file_list_cur");
	FillAdminList(1, "file_list_all");
	FillAdminList(4, "file_list_super");
	FillAdminList(5, "file_list_missing");
}



static string FD2Str(int k)
{
	if (k==-1)				return _("[Engine]");
	if (k==FD_MODEL)			return _("Model");
	if (k==FD_TEXTURE)		return _("Texture");
	if (k==FD_SOUND)			return _("Sound");
	if (k==FD_MATERIAL)		return _("Material");
	if (k==FD_TERRAIN)		return _("Terrain");
	if (k==FD_WORLD)			return _("World");
	if (k==FD_SHADERFILE)	return _("Shader");
	if (k==FD_FONT)			return _("Font");
	if (k==FD_SCRIPT)		return _("Script");
	if (k==FD_CAMERAFLIGHT)	return _("Camera");
	if (k==FD_FILE)			return _("File");
	return "???";
}

void AdministrationDialog::FillAdminList(int view, const string &lid)
{
	reset(lid);
	string sep = hui::separator;
	hui::separator = "::";

	// currently viewed list
	AdminFileList *l = get_list(lid);
	assert(l);

	l->clear();
	if (view == 0){ // current game (in game.ini)
		l->add_recursive((*data->file_list)[0]);
	}else if (view == 1){ // all files
		*l = *data->file_list;
	}else if (view == 2){ // selected file
		for (AdminFile *a: SelectedAdminFile->Parent)
			l->add(a);
	}else if (view == 3){
		for (AdminFile *a: SelectedAdminFile->Child)
			l->add(a);
	}else if (view == 4){ // unnessecary
		for (AdminFile *a: *data->file_list)
			if ((a->Kind >= 0) && (a->Parent.num == 0))
				l->add(a);
	}else if (view == 5){ // missing
		for (AdminFile *a: *data->file_list)
			if (a->Missing)
				l->add(a);
	}


	l->sort();

	// place them into the list
	int k=-2;
	for (AdminFile *a: *l){
		add_string(lid, format("%s::%s::%d::%d::%s",
			(k != a->Kind) ? FD2Str(a->Kind).c_str() : "",
			a->Name.c_str(),
			a->Parent.num, a->Child.num,
			a->Missing ? "1" : "0"));
		k = a->Kind;
	}
#ifdef NIX_OS_WINDOWS
	AddString(lid, ""); // windows ... -> doesn't rescale the list's columns automatically  m(-_-)m
#endif
	hui::separator = sep;
}

void AdministrationDialog::ShowDetail(int n, const string &lid)
{
	AdminFileList *l = get_list(lid);
	assert(l);
	assert(n >= 0);
	assert(n < l->num);
	SelectedAdminFile = (*l)[n];
	set_string("file_details", SelectedAdminFile->Name.str());
	FillAdminList(2, "file_list_detail_source");
	FillAdminList(3, "file_list_detail_dest");
	set_int("ad_tab_control", 2);
}


void AdministrationDialog::OnExportGame() {
	data->session->mode_admin->export_game();
}

AdminFileList *AdministrationDialog::get_list(const string &lid) {
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
	Array<int> index = get_selection(lid);
	AdminFileList *l = get_list(lid);
	assert(l);
	for (int i: index)
		r.add((*l)[i]);
	return r;
}

Array<AdminFile*> AdministrationDialog::GetSelectedFiles()
{
	int page = get_int("ad_tab_control");
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
	data->session->error(_("Please select exactly one file!"));
	return nullptr;
}

void AdministrationDialog::OnClose()
{
	data->session->set_mode(data->session->mode_model);
}

void AdministrationDialog::OnExit()
{	data->session->set_mode(data->session->mode_model);	}

void AdministrationDialog::OnRename()
{}//{	data->Rename();	}

void AdministrationDialog::OnDelete()
{}//{	madmin->Delete();	}

void AdministrationDialog::OnEdit() {
	AdminFile *a = GetSingleSelectedFile();
	if (!a)
		return;
	data->session->universal_edit(a->Kind, a->Name, true);
}

void AdministrationDialog::OnFileList() {
	string id = hui::get_event()->id;
	int n = get_int(id);
	ShowDetail(n, id);
}

void AdministrationDialog::OnRudimentaryConfiguration() {
	data->session->mode_admin->basic_settings();
}

