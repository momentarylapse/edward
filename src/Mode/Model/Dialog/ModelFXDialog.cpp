/*
 * ModelFXDialog.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ModelFXDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../lib/script/script.h"
#include "../../../Edward.h"

ModelFXDialog::ModelFXDialog(CHuiWindow* _parent, bool _allow_parent, DataModel* _data, int _type, int _index) :
CHuiWindow("dummy", -1, -1, 230, 400, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;
	index = _index;
	if (index >= 0)
		temp = data->Fx[index];
	else{
		temp.clear();
		temp.Kind = _type;
	}

	// dialog
	FromResource("fx_dialog");

	EventM("hui:close", this, &ModelFXDialog::OnClose);
	EventM("cancel", this, &ModelFXDialog::OnClose);
	EventM("ok", this, &ModelFXDialog::OnOk);
	EventM("search", this, &ModelFXDialog::OnFindSoundFile);
	EventM("find_scriptfile", this, &ModelFXDialog::OnFindScriptFile);

	LoadData();
}

ModelFXDialog::~ModelFXDialog()
{
}

void ModelFXDialog::LoadData()
{
	if (temp.Kind == FXKindScript){
		SetInt("fx_tab_control", 0);
		SetString("script_file", temp.File);
	}else if (temp.Kind == FXKindLight){
		SetInt("fx_tab_control", 1);
		SetColor("mat_am", temp.Colors[0]);
		SetColor("mat_di", temp.Colors[1]);
		SetColor("mat_sp", temp.Colors[2]);
		SetFloat("light_radius", temp.Size);
	}else if (temp.Kind == FXKindSound){
		SetInt("fx_tab_control", 2);
		SetString("filename", temp.File);
		SetFloat("sound_radius", temp.Size);
		SetFloat("speed", temp.Speed * 100.0f);
	}else if (temp.Kind == FXKindForceField){
		SetInt("fx_tab_control", 3);
		SetFloat("forcefield_radius", temp.Size);
		SetFloat("intensity", temp.Intensity);
		SetInt("formula", temp.InvQuad ? 1 : 0);
	}
}

void ModelFXDialog::ApplyData()
{
	if (temp.Kind == FXKindScript){
		temp.File = GetString("script_file");
	}else if (temp.Kind == FXKindLight){
		temp.Colors[0] = GetColor("mat_am");
		temp.Colors[1] = GetColor("mat_di");
		temp.Colors[2] = GetColor("mat_sp");
		temp.Size = GetFloat("light_radius");
	}else if (temp.Kind == FXKindSound){
		temp.File = GetString("filename");
		temp.Size = GetFloat("sound_radius");
		temp.Speed = GetFloat("speed") * 0.01f;
	}else if (temp.Kind == FXKindForceField){
		temp.Size = GetFloat("forcefield_radius");
		temp.Intensity = GetFloat("intensity");
		temp.InvQuad = (GetInt("formula")  == 1);
	}

	if (index >= 0)
		data->EditEffect(index, temp);
	else
		data->SelectionAddEffects(temp);
}

void ModelFXDialog::OnFindScriptFile()
{
	if (ed->FileDialog(FDScript,false,true)){
		string filename = ed->DialogFile;
		SetString("script_file", ed->DialogFile);

		Script::Directory = ScriptDir;
		Script::Script *s = new Script::Script(filename, true); // just analyse
		Script::PreScript *ps = s->pre_script;
		if (s->Error){
			ed->ErrorBox(format(_("Fehler in Script-Datei: \"%s\"\n%s\n%s"), filename.c_str(), ps->ErrorMsgExt[0].c_str(), ps->ErrorMsgExt[1].c_str()));
			msg_db_l(1);
			return;
		}
		if (!s->MatchFunction("OnEffectCreate", "void", 1, "effect"))
			ed->ErrorBox(_("Script-Datei enth&alt keine Funktion \"void OnEffectCreate( effect )\""));
		else if (!s->MatchFunction("OnEffectIterate", "void", 1, "effect"))
			ed->ErrorBox(_("Script-Datei enth&alt keine Funktion \"void OnEffectIterate( effect )\""));
	}
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


