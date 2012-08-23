/*
 * AdministrationDialog.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "AdministrationDialog.h"
#include "../../../Data/Administration/DataAdministration.h"
#include "../../../Edward.h"
#include "../../../Mode/Welcome/ModeWelcome.h"

AdministrationDialog::AdministrationDialog(CHuiWindow* _parent, bool _allow_parent, DataAdministration *_data):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	data = _data;

	// dialog
	FromResource("ad_dialog");
	EventM("hui:close", this, (void(HuiEventHandler::*)())&AdministrationDialog::OnClose);
	/*EventM("cancel", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnClose);
	EventM("set", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::ApplyData);
	EventM("ok", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnOk);
	EventM("mat_add_texture_level", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnAddTextureLevel);
	EventM("mat_textures", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnTextures);
	EventMX("mat_textures", "hui:select", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnTexturesSelect);
	EventM("mat_delete_texture_level", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnDeleteTextureLevel);
	EventM("mat_empty_texture_level", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnEmptyTextureLevel);
	EventM("transparency_mode", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnTransparencyMode);
	EventM("reflection", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnReflection);
	EventM("reflection_textures", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnReflectionTextures);
	EventM("find_effect", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnFindEffect);*/

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
	FillAdminList(3, "file_list_super");
	FillAdminList(4, "file_list_missing");
}

void AdministrationDialog::OnUpdate(Observable* o)
{
	LoadData();
}



void AdministrationDialog::FillAdminList(int view, const string &lid)
{
	msg_db_r("FillAdminList",1);
	if (view<0)
		view = GetInt("ad_tab_control");

	Reset(lid);
	string sep = HuiComboBoxSeparator;
	HuiComboBoxSeparator = "::";

	// currently viewed list
	AdminFileList *l = get_list(lid);
	l->clear();
	if (view==0){ // current game (in game.ini)
		data->FindRecursive(*l, data->file_list_all[0], false, -1);
		data->FindRecursive(*l, data->file_list_all[1], false, -1);
		data->FindRecursive(*l, data->file_list_all[2], false, -1);
	}else if (view==1){ // all files
		*l = data->file_list_all;
	}else if (view==2){ // selected file
		if (lid == "file_list_detail_source"){
			for (int j=0;j<SelectedAdminFile->Source.num;j++)
				l->add(SelectedAdminFile->Source[j]);
		}else{
			for (int j=0;j<SelectedAdminFile->Dest.num;j++)
				l->add(SelectedAdminFile->Dest[j]);
		}
	}else if (view==3){ // unnessecary
		foreach(data->file_list_all, a)
			if ((a->Kind >= 0) && (a->Source.num == 0))
				l->add(a);
	}else if (view==4){ // missing
		foreach(data->file_list_all, a)
			if (a->Missing)
				l->add(a);
	}


	data->SortList(*l);

	// place them into the list
	int k=-2;
	foreach(*l, a){
		AddString(lid, format("%s::%s::%d::%d::%s",
			(k != a->Kind) ? data->FD2Str(a->Kind).c_str() : "",
			a->Name.c_str(),
			a->Source.num, a->Dest.num,
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
	SelectedAdminFile = (*l)[n];
	SetString("file_details", SelectedAdminFile->Name);
	FillAdminList(2, "file_list_detail_source");
	FillAdminList(2, "file_list_detail_dest");
	SetInt("ad_tab_control", 2);
	msg_db_l(1);
}


AdminFileList *AdministrationDialog::get_list(const string &lid)
{
	if (lid == "file_list_cur")
		return &file_list_cur;
	if (lid == "file_list_all")
		return &file_list_all;
	if (lid == "file_list_detail_source")
		return &file_list_detail_source;
	if (lid == "file_list_detail_dest")
		return &file_list_detail_dest;
	if (lid == "file_list_super")
		return &file_list_super;
	if (lid == "file_list_missing")
		return &file_list_missing;
	return NULL;
}

void AdministrationDialog::OnClose()
{
	ed->SetMode(mode_welcome);
}

