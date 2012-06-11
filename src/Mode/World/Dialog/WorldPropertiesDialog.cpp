/*
 * WorldPropertiesDialog.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "WorldPropertiesDialog.h"
#include "../../../Edward.h"
#include "../ModeWorld.h"
#include "../../../Action/World/ActionWorldEditData.h"

WorldPropertiesDialog::WorldPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("world_dialog");

	EventM("cancel", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnClose);
	EventM("apply", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::ApplyData);
	EventM("ok", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnOk);

	EventM("sun_enabled", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnSunEnabled);
	EventM("fog_enabled", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnFogEnabled);
	EventM("skybox", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnSkybox);
	EventM("remove_skybox", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnRemoveSkybox);
	EventM("max_script_vars", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnMaxScriptVars);
	EventMX("script_vars", "hui:change", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnScriptVarEdit);
	//EventM("model_script_var_template", this, (void(HuiEventHandler::*)())&ModelPropertiesDialog::OnModelScriptVarTemplate);

	Subscribe(data);

	temp = data->meta_data;
	LoadData();
}

WorldPropertiesDialog::~WorldPropertiesDialog()
{
	mode_world->WorldDialog = NULL;
}

void WorldPropertiesDialog::OnSkybox()
{
	int n = GetInt("");
	if (ed->FileDialog(FDModel,false,true)){
		temp.SkyBoxFile[n] = ed->DialogFileNoEnding;
		FillSkyboxList();
	}
}



void WorldPropertiesDialog::OnClose()
{
	delete(this);
}



void WorldPropertiesDialog::OnSunEnabled()
{
	bool b = IsChecked("");
	Enable("sun_am", b);
	Enable("sun_di", b);
	Enable("sun_sp", b);
	Enable("sun_ang_x", b);
	Enable("sun_ang_y", b);
}



void WorldPropertiesDialog::OnRemoveSkybox()
{
	int n = GetInt("skybox");
	if (n >= 0)
		if (temp.SkyBoxFile[n].num > 0){
			temp.SkyBoxFile[n] = "";
			FillSkyboxList();
		}
}



void WorldPropertiesDialog::OnFogEnabled()
{
	bool b = IsChecked("");
	Enable("fog_mode", b);
	Enable("fog_start", b);
	Enable("fog_end", b);
	Enable("fog_density", b);
	Enable("fog_color", b);
}



void WorldPropertiesDialog::FillSkyboxList()
{
	HuiComboBoxSeparator=':';
	Reset("skybox");
	for (int i=0;i<temp.SkyBoxFile.num;i++)
		AddString("skybox",format("%d:%s",i,temp.SkyBoxFile[i].c_str()));
	HuiComboBoxSeparator='\\';
}



void WorldPropertiesDialog::OnUpdate(Observable *o)
{
	temp = data->meta_data;
	LoadData();
}



void WorldPropertiesDialog::OnScriptVarEdit()
{
	int row = HuiGetEvent()->row;
	temp.ScriptVar[row] = s2f(GetCell("script_vars", row, 2));
}



void WorldPropertiesDialog::FillScriptVarList()
{
	Reset("script_vars");
	foreachi(temp.ScriptVar, v, i)
		/*if (i<NumObjectScriptVarNames)
			AddString("script_vars", format("%d\\%s\\%.6f", i, ObjectScriptVarName[i].c_str(), v));
		else*/
			AddString("script_vars", format("%d\\\\%.6f", i, v));
}



void WorldPropertiesDialog::ApplyData()
{
	temp.Gravity.x = GetFloat("gravitation_x");
	temp.Gravity.y = GetFloat("gravitation_y");
	temp.Gravity.z = GetFloat("gravitation_z");
	temp.BackGroundColor = GetColor("bgc");
	temp.FogEnabled = IsChecked("fog_enabled");
	temp.FogMode = GetInt("fog_mode");
	temp.FogStart = GetFloat("fog_start");
	temp.FogEnd = GetFloat("fog_end");
	temp.FogDensity = GetFloat("fog_density");
	temp.FogColor = GetColor("fog_color");

	temp.SunEnabled = IsChecked("sun_enabled");
	temp.SunAmbient = GetColor("sun_am");
	temp.SunDiffuse = GetColor("sun_di");
	temp.SunSpecular = GetColor("sun_sp");
	temp.SunAng.x = GetFloat("sun_ang_x") / 180.0f * pi;
	temp.SunAng.y = GetFloat("sun_ang_y") / 180.0f * pi;
	temp.Ambient = GetColor("ambient");

	data->Execute(new ActionWorldEditData(temp));
}



void WorldPropertiesDialog::OnScriptVarTemplate()
{
}



void WorldPropertiesDialog::OnMaxScriptVars()
{
	temp.ScriptVar.resize(GetInt("max_script_vars"));
	FillScriptVarList();
}



void WorldPropertiesDialog::OnOk()
{
	ApplyData();
	delete(this);
}



void WorldPropertiesDialog::LoadData()
{
	SetFloat("gravitation_x", temp.Gravity.x);
	SetFloat("gravitation_y", temp.Gravity.y);
	SetFloat("gravitation_z", temp.Gravity.z);
	SetColor("bgc", temp.BackGroundColor);
	Check("fog_enabled", temp.FogEnabled);
	SetInt("fog_mode", temp.FogMode);
	SetFloat("fog_start", temp.FogStart);
	SetFloat("fog_end", temp.FogEnd);
	SetFloat("fog_density", temp.FogDensity);
	SetColor("fog_color", temp.FogColor);
	Enable("fog_mode", temp.FogEnabled);
	Enable("fog_start", temp.FogEnabled);
	Enable("fog_end", temp.FogEnabled);
	Enable("fog_density", temp.FogEnabled);
	Enable("fog_color", temp.FogEnabled);

	Check("sun_enabled", temp.SunEnabled);
	SetColor("sun_am", temp.SunAmbient);
	SetColor("sun_di", temp.SunDiffuse);
	SetColor("sun_sp", temp.SunSpecular);
	SetFloat("sun_ang_x", temp.SunAng.x * 180.0f / pi);
	SetFloat("sun_ang_y", temp.SunAng.y * 180.0f / pi);
	Enable("sun_am", temp.SunEnabled);
	Enable("sun_di", temp.SunEnabled);
	Enable("sun_sp", temp.SunEnabled);
	Enable("sun_ang_x", temp.SunEnabled);
	Enable("sun_ang_y", temp.SunEnabled);
	SetColor("ambient", temp.Ambient);

	SetInt("max_script_vars", temp.ScriptVar.num);

	FillSkyboxList();
	FillScriptVarList();
}


