/*
 * ModelFXDialog.cpp
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#include "ModelFXDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../lib/kaba/kaba.h"
#include "../../../Edward.h"

ModelFXDialog::ModelFXDialog(hui::Window* _parent, bool _allow_parent, DataModel* _data, int _type, int _index) :
	hui::Dialog("fx_dialog", 400, 300, _parent, _allow_parent)
{
	fromResource("fx_dialog");
	data = _data;
	index = _index;
	if (index >= 0)
		temp = data->fx[index];
	else{
		temp.clear();
		temp.type = _type;
	}

	event("hui:close", std::bind(&ModelFXDialog::OnClose, this));
	event("cancel", std::bind(&ModelFXDialog::OnClose, this));
	event("ok", std::bind(&ModelFXDialog::OnOk, this));
	event("search", std::bind(&ModelFXDialog::OnFindSoundFile, this));
	event("find_scriptfile", std::bind(&ModelFXDialog::OnFindScriptFile, this));

	LoadData();
}

ModelFXDialog::~ModelFXDialog()
{
}

void ModelFXDialog::LoadData()
{
	if (temp.type == FX_TYPE_SCRIPT){
		setInt("fx_tab_control", 0);
		setString("script_file", temp.file);
	}else if (temp.type == FX_TYPE_LIGHT){
		setInt("fx_tab_control", 1);
		setColor("mat_am", temp.colors[0]);
		setColor("mat_di", temp.colors[1]);
		setColor("mat_sp", temp.colors[2]);
		setFloat("light_radius", temp.size);
	}else if (temp.type == FX_TYPE_SOUND){
		setInt("fx_tab_control", 2);
		setString("filename", temp.file);
		setFloat("sound_radius", temp.size);
		setFloat("speed", temp.speed * 100.0f);
	}else if (temp.type == FX_TYPE_FORCEFIELD){
		setInt("fx_tab_control", 3);
		setFloat("forcefield_radius", temp.size);
		setFloat("intensity", temp.intensity);
		setInt("formula", temp.inv_quad ? 1 : 0);
	}
}

void ModelFXDialog::ApplyData()
{
	if (temp.type == FX_TYPE_SCRIPT){
		temp.file = getString("script_file");
	}else if (temp.type == FX_TYPE_LIGHT){
		temp.colors[0] = getColor("mat_am");
		temp.colors[1] = getColor("mat_di");
		temp.colors[2] = getColor("mat_sp");
		temp.size = getFloat("light_radius");
	}else if (temp.type == FX_TYPE_SOUND){
		temp.file = getString("filename");
		temp.size = getFloat("sound_radius");
		temp.speed = getFloat("speed") * 0.01f;
	}else if (temp.type == FX_TYPE_FORCEFIELD){
		temp.size = getFloat("forcefield_radius");
		temp.intensity = getFloat("intensity");
		temp.inv_quad = (getInt("formula")  == 1);
	}

	if (index >= 0)
		data->editEffect(index, temp);
	else
		data->selectionAddEffects(temp);
}

void ModelFXDialog::OnFindScriptFile()
{
	if (ed->fileDialog(FD_SCRIPT,false,true)){
		string filename = ed->dialog_file;
		setString("script_file", ed->dialog_file);

		try{
			Kaba::Script *s = Kaba::Load(filename, true); // just analyse
			if (!s->MatchFunction("OnEffectCreate", "void", 1, "effect"))
				ed->errorBox(_("Script-Datei enth&alt keine Funktion \"void OnEffectCreate( effect )\""));
			else if (!s->MatchFunction("OnEffectIterate", "void", 1, "effect"))
				ed->errorBox(_("Script-Datei enth&alt keine Funktion \"void OnEffectIterate( effect )\""));
		}catch(Kaba::Exception &e){
			ed->errorBox(_("Fehler in Script-Datei: ") + e.message);
		}
	}
}

void ModelFXDialog::OnFindSoundFile()
{
	if (ed->fileDialog(FD_SOUND,false,true))
		setString("filename", ed->dialog_file);
}

void ModelFXDialog::OnClose()
{
	destroy();
}

void ModelFXDialog::OnOk()
{
	ApplyData();
	destroy();
}


