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

ModelFXDialog::ModelFXDialog(HuiWindow* _parent, bool _allow_parent, DataModel* _data, int _type, int _index) :
	HuiWindow("fx_dialog", _parent, _allow_parent)
{
	data = _data;
	index = _index;
	if (index >= 0)
		temp = data->Fx[index];
	else{
		temp.clear();
		temp.Kind = _type;
	}

	event("hui:close", this, &ModelFXDialog::OnClose);
	event("cancel", this, &ModelFXDialog::OnClose);
	event("ok", this, &ModelFXDialog::OnOk);
	event("search", this, &ModelFXDialog::OnFindSoundFile);
	event("find_scriptfile", this, &ModelFXDialog::OnFindScriptFile);

	LoadData();
}

ModelFXDialog::~ModelFXDialog()
{
}

void ModelFXDialog::LoadData()
{
	if (temp.Kind == FXKindScript){
		setInt("fx_tab_control", 0);
		setString("script_file", temp.File);
	}else if (temp.Kind == FXKindLight){
		setInt("fx_tab_control", 1);
		setColor("mat_am", temp.Colors[0]);
		setColor("mat_di", temp.Colors[1]);
		setColor("mat_sp", temp.Colors[2]);
		setFloat("light_radius", temp.Size);
	}else if (temp.Kind == FXKindSound){
		setInt("fx_tab_control", 2);
		setString("filename", temp.File);
		setFloat("sound_radius", temp.Size);
		setFloat("speed", temp.Speed * 100.0f);
	}else if (temp.Kind == FXKindForceField){
		setInt("fx_tab_control", 3);
		setFloat("forcefield_radius", temp.Size);
		setFloat("intensity", temp.Intensity);
		setInt("formula", temp.InvQuad ? 1 : 0);
	}
}

void ModelFXDialog::ApplyData()
{
	if (temp.Kind == FXKindScript){
		temp.File = getString("script_file");
	}else if (temp.Kind == FXKindLight){
		temp.Colors[0] = getColor("mat_am");
		temp.Colors[1] = getColor("mat_di");
		temp.Colors[2] = getColor("mat_sp");
		temp.Size = getFloat("light_radius");
	}else if (temp.Kind == FXKindSound){
		temp.File = getString("filename");
		temp.Size = getFloat("sound_radius");
		temp.Speed = getFloat("speed") * 0.01f;
	}else if (temp.Kind == FXKindForceField){
		temp.Size = getFloat("forcefield_radius");
		temp.Intensity = getFloat("intensity");
		temp.InvQuad = (getInt("formula")  == 1);
	}

	if (index >= 0)
		data->EditEffect(index, temp);
	else
		data->SelectionAddEffects(temp);
}

void ModelFXDialog::OnFindScriptFile()
{
	if (ed->fileDialog(FDScript,false,true)){
		string filename = ed->DialogFile;
		setString("script_file", ed->DialogFile);

		try{
			Script::Script *s = Script::Load(filename, true); // just analyse
			if (!s->MatchFunction("OnEffectCreate", "void", 1, "effect"))
				ed->errorBox(_("Script-Datei enth&alt keine Funktion \"void OnEffectCreate( effect )\""));
			else if (!s->MatchFunction("OnEffectIterate", "void", 1, "effect"))
				ed->errorBox(_("Script-Datei enth&alt keine Funktion \"void OnEffectIterate( effect )\""));
		}catch(Script::Exception &e){
			ed->errorBox(_("Fehler in Script-Datei: ") + e.message);
		}
	}
}

void ModelFXDialog::OnFindSoundFile()
{
	if (ed->fileDialog(FDSound,false,true))
		setString("filename", ed->DialogFile);
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


