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

#define WorldPhysicsDec			3
#define WorldLightDec			1
#define WorldFogDec				6

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
	EventM("sun_ang_from_camera", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnSunAngFromCamera);
	EventM("fog_enabled", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnFogEnabled);
	EventM("skybox", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnSkybox);
	EventMX("skybox", "hui:select", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnSkyboxSelect);
	EventM("remove_skybox", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnRemoveSkybox);
	EventM("physics_enabled", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnPhysicsEnabled);
	EventMX("script_list", "hui:select", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnScriptSelect);
	EventM("remove_script", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnRemoveScript);
	EventM("add_script", this, (void(HuiEventHandler::*)())&WorldPropertiesDialog::OnAddScript);
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
	Unsubscribe(data);
}

void WorldPropertiesDialog::OnSkybox()
{
	int n = GetInt("");
	if (ed->FileDialog(FDModel,false,true)){
		temp.SkyBoxFile[n] = ed->DialogFileNoEnding;
		FillSkyboxList();
	}
}

void WorldPropertiesDialog::OnSkyboxSelect()
{
	int row = GetInt("");
	if (row >= 0)
		Enable("remove_skybox", temp.SkyBoxFile[row].num > 0);
	else
		Enable("remove_skybox", false);
}


void WorldPropertiesDialog::OnScriptSelect()
{
	int row = GetInt("");
	Enable("remove_script", row >= 0);
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
	Enable("sun_ang_from_camera", b);
}


void WorldPropertiesDialog::OnSunAngFromCamera()
{
	SetFloat("sun_ang_x", ed->multi_view_3d->ang.x * 180.0f / pi);
	SetFloat("sun_ang_y", ed->multi_view_3d->ang.y * 180.0f / pi);
}


void WorldPropertiesDialog::OnPhysicsEnabled()
{
	bool b = IsChecked("");
	Enable("gravitation_x", b);
	Enable("gravitation_y", b);
	Enable("gravitation_z", b);
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



void WorldPropertiesDialog::OnAddScript()
{
	if (ed->FileDialog(FDScript, false, true)){
		ModeWorldScript s;
		s.Filename = ed->DialogFileComplete.substr(ScriptDir.num, -1);
		temp.Script.add(s);
		FillScriptList();
	}
}



void WorldPropertiesDialog::OnRemoveScript()
{
	int n = GetInt("script_list");
	if (n >= 0){
		temp.Script.erase(n);
		FillScriptList();
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
	HuiComboBoxSeparator = ':';
	Reset("skybox");
	foreachi(temp.SkyBoxFile, sb, i)
		AddString("skybox", format("%d:%s", i, sb.c_str()));
	HuiComboBoxSeparator = '\\';
	Enable("remove_skybox", false);
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



void WorldPropertiesDialog::FillScriptList()
{
	HuiComboBoxSeparator = ':';
	Reset("script_list");
	foreach(temp.Script, s)
		AddString("script_list", s.Filename);
	Enable("remove_script", false);
	HuiComboBoxSeparator = '\\';
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
	SetDecimals(WorldFogDec);
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

	SetDecimals(WorldLightDec);
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
	Enable("sun_ang_from_camera", temp.SunEnabled);
	SetColor("ambient", temp.Ambient);

	SetDecimals(WorldPhysicsDec);
	Check("physics_enabled", temp.PhysicsEnabled);
	Enable("gravitation_x", temp.PhysicsEnabled);
	Enable("gravitation_y", temp.PhysicsEnabled);
	Enable("gravitation_z", temp.PhysicsEnabled);
	SetFloat("gravitation_x", temp.Gravity.x);
	SetFloat("gravitation_y", temp.Gravity.y);
	SetFloat("gravitation_z", temp.Gravity.z);

	SetInt("max_script_vars", temp.ScriptVar.num);

	FillSkyboxList();
	FillScriptVarList();
	FillScriptList();
}


