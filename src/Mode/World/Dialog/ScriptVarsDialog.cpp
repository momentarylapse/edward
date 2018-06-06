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
	fromResource("script_vars_dialog");
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
		data->variables[i].value = getCell("variables", i, 2);
}



void ScriptVarsDialog::LoadData()
{
	reset("variables");
	for (auto &v: data->variables)
		addString("variables", v.name + "\\" + v.type + "\\" + v.value);
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

