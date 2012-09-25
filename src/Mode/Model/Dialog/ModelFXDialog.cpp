/*
 * ModelFXDialog.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ModelFXDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelFXDialog::ModelFXDialog(CHuiWindow* _parent, bool _allow_parent, DataModel* _data, int _type) :
CHuiWindow("dummy", -1, -1, 230, 400, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;
	temp.Kind = _type;

	// dialog
	FromResource("fx_dialog");

	EventM("hui:close", this, (void(HuiEventHandler::*)())&ModelFXDialog::OnClose);
	EventM("cancel", this, (void(HuiEventHandler::*)())&ModelFXDialog::OnClose);
	EventM("ok", this, (void(HuiEventHandler::*)())&ModelFXDialog::OnOk);
	EventM("search", this, (void(HuiEventHandler::*)())&ModelFXDialog::OnFindSoundFile);
	EventM("find_scriptfile", this, (void(HuiEventHandler::*)())&ModelFXDialog::OnFindScriptFile);

	LoadData();
}

ModelFXDialog::~ModelFXDialog()
{
}

void ModelFXDialog::LoadData()
{
	if (temp.Kind == FXKindScript)
		SetInt("fx_tab_control", 0);
	else if (temp.Kind == FXKindLight)
		SetInt("fx_tab_control", 1);
	else if (temp.Kind == FXKindSound)
		SetInt("fx_tab_control", 2);
	else if (temp.Kind == FXKindForceField)
		SetInt("fx_tab_control", 3);
}

void ModelFXDialog::ApplyData()
{
	if (temp.Kind == FXKindScript){
		temp.File = GetString("script_file");
		temp.Function = GetString("script_function");
	}else if (temp.Kind == FXKindLight){
		temp.Colors[0] = GetColor("mat_am");
		temp.Colors[1] = GetColor("mat_di");
		temp.Colors[2] = GetColor("mat_sp");
		temp.Size = GetInt("radius");
	}else if (temp.Kind == FXKindSound){
		temp.File = GetString("filename");
		temp.Size = GetInt("sound_radius");
		temp.Speed = GetInt("speed");
	}else if (temp.Kind == FXKindForceField){
	}

	data->SelectionAddEffects(temp);
}

void ModelFXDialog::OnFindScriptFile()
{
	if (ed->FileDialog(FDScript,false,true))
		{}//SetString("filename", ed->DialogFile);
}

void ModelFXDialog::OnFindSoundFile()
{
	if (ed->FileDialog(FDSound,false,true))
		SetString("filename", ed->DialogFile);
}

void ModelFXDialog::OnClose()
{
	delete(this);
}

void ModelFXDialog::OnOk()
{
	ApplyData();
	delete(this);
}


