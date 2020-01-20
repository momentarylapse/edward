/*
 * ScriptVarsDialog.cpp
 *
 *  Created on: 05.06.2018
 *      Author: michi
 */

#include "ScriptVarsDialog.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../lib/kaba/kaba.h"

ScriptVarsDialog::ScriptVarsDialog(hui::Window *_parent, WorldScript *_data) :
	hui::Dialog("script_vars_dialog", 400, 300, _parent, false)
{
	from_resource("script_vars_dialog");
	data = _data;

	event("cancel", std::bind(&ScriptVarsDialog::OnClose, this));
	event("hui:close", std::bind(&ScriptVarsDialog::OnClose, this));
	event("ok", std::bind(&ScriptVarsDialog::OnOk, this));

	LoadData();
}

ScriptVarsDialog::~ScriptVarsDialog()
{
}

void ScriptVarsDialog::ApplyData()
{
	for (int i=0; i<data->variables.num; i++)
		data->variables[i].value = get_cell("variables", i, 2);
}



void ScriptVarsDialog::LoadData()
{
	set_string("class", _("Class: ") + data->class_name);
	if (data->class_name == "")
		set_string("class", "- no class derived from Controller found -");

	reset("variables");
	for (auto &v: data->variables)
		add_string("variables", v.name + "\\" + v.type + "\\" + v.value);
	enable("variables", data->variables.num > 0);
}

void ScriptVarsDialog::OnClose()
{
	destroy();
}

void ScriptVarsDialog::OnOk()
{
	ApplyData();
	destroy();
}

