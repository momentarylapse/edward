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
#include "../../../Storage/Storage.h"

ModelFXDialog::ModelFXDialog(hui::Window* _parent, bool _allow_parent, DataModel* _data, int _type, int _index) :
	hui::Dialog("fx_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("fx_dialog");
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
		set_int("fx_tab_control", 0);
		set_string("script_file", temp.file.str());
	}else if (temp.type == FX_TYPE_LIGHT){
		set_int("fx_tab_control", 1);
		set_color("mat_am", temp.colors[0]);
		set_color("mat_di", temp.colors[1]);
		set_color("mat_sp", temp.colors[2]);
		set_float("light_radius", temp.size);
	}else if (temp.type == FX_TYPE_SOUND){
		set_int("fx_tab_control", 2);
		set_string("filename", temp.file.str());
		set_float("sound_radius", temp.size);
		set_float("speed", temp.speed * 100.0f);
	}else if (temp.type == FX_TYPE_FORCEFIELD){
		set_int("fx_tab_control", 3);
		set_float("forcefield_radius", temp.size);
		set_float("intensity", temp.intensity);
		set_int("formula", temp.inv_quad ? 1 : 0);
	}
}

void ModelFXDialog::ApplyData()
{
	if (temp.type == FX_TYPE_SCRIPT){
		temp.file = get_string("script_file");
	}else if (temp.type == FX_TYPE_LIGHT){
		temp.colors[0] = get_color("mat_am");
		temp.colors[1] = get_color("mat_di");
		temp.colors[2] = get_color("mat_sp");
		temp.size = get_float("light_radius");
	}else if (temp.type == FX_TYPE_SOUND){
		temp.file = get_string("filename");
		temp.size = get_float("sound_radius");
		temp.speed = get_float("speed") * 0.01f;
	}else if (temp.type == FX_TYPE_FORCEFIELD){
		temp.size = get_float("forcefield_radius");
		temp.intensity = get_float("intensity");
		temp.inv_quad = (get_int("formula")  == 1);
	}

	if (index >= 0)
		data->editEffect(index, temp);
	else
		data->selectionAddEffects(temp);
}

void ModelFXDialog::OnFindScriptFile() {
	storage->file_dialog(FD_SCRIPT,false,true, [this] {
		Path filename = storage->dialog_file;
		set_string("script_file", storage->dialog_file.str());

		try {
			auto c = ownify(kaba::Context::create());
			auto s = c->load_module(filename, true); // just analyse
			if (!s->match_function("OnEffectCreate", "void", {"effect"}))
				ed->error_box(_("Script file does not contain a function \"void OnEffectCreate( effect )\""));
			else if (!s->match_function("OnEffectIterate", "void", {"effect"}))
				ed->error_box(_("Script file does not contain a function \"void OnEffectIterate( effect )\""));
		} catch(kaba::Exception &e) {
			ed->error_box(_("Error in script file:") + e.message());
		}
	});
}

void ModelFXDialog::OnFindSoundFile() {
	storage->file_dialog(FD_SOUND,false,true, [this] {
		set_string("filename", storage->dialog_file.str());
	});
}

void ModelFXDialog::OnClose() {
	request_destroy();
}

void ModelFXDialog::OnOk() {
	ApplyData();
	request_destroy();
}


