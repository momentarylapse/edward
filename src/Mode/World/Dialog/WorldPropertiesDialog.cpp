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
#include "../../../lib/script/script.h"

#define WorldPhysicsDec			3
#define WorldLightDec			1
#define WorldFogDec				6

WorldPropertiesDialog::WorldPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("world_dialog");

	SetTooltip("bgc", _("Farbe des Himmels"));
	SetTooltip("skybox", _("Modelle, die &uber die Hintergrundfarge gemalt werden\n- Doppelklick um ein Modell zu w&ahlen"));

	SetTooltip("fog_start", _("Abstand, ab dem der Nebel beginnt (Intensit&at 0)"));
	SetTooltip("fog_end", _("maximale Sichtweite, dahinter hat der Nebel volle Intensit&at"));
	SetTooltip("fog_density", _("Nebelintensit&at = exp( - Dichte * Entfernung )"));

	SetTooltip("ambient", _("Generelles Umgebungslicht zus&atzlich zu allen Lichtquellen"));
	SetTooltip("sun_am", _("ungerichtetes Umgebungslicht"));
	SetTooltip("sun_di", _("frontales Licht"));
	SetTooltip("sun_sp", _("Glanzlicht"));
	SetTooltip("sun_ang_x", _("H&ohe &uber dem Horizont"));
	SetTooltip("sun_ang_y", _("Kompassrichtung entlang des Horizonts"));

	EventM("cancel", this, &WorldPropertiesDialog::OnClose);
	EventM("hui:close", this, &WorldPropertiesDialog::OnClose);
	EventM("apply", this, &WorldPropertiesDialog::ApplyData);
	EventM("ok", this, &WorldPropertiesDialog::OnOk);

	EventM("sun_enabled", this, &WorldPropertiesDialog::OnSunEnabled);
	EventM("sun_ang_from_camera", this, &WorldPropertiesDialog::OnSunAngFromCamera);
	EventM("fog_mode:none", this, &WorldPropertiesDialog::OnFogModeNone);
	EventM("fog_mode:linear", this, &WorldPropertiesDialog::OnFogModeLinear);
	EventM("fog_mode:exp", this, &WorldPropertiesDialog::OnFogModeExp);
	EventM("fog_mode:exp2", this, &WorldPropertiesDialog::OnFogModeExp);
	EventM("skybox", this, &WorldPropertiesDialog::OnSkybox);
	EventMX("skybox", "hui:select", this, &WorldPropertiesDialog::OnSkyboxSelect);
	EventM("remove_skybox", this, &WorldPropertiesDialog::OnRemoveSkybox);
	EventM("physics_enabled", this, &WorldPropertiesDialog::OnPhysicsEnabled);
	EventMX("script_list", "hui:select", this, &WorldPropertiesDialog::OnScriptSelect);
	EventM("remove_script", this, &WorldPropertiesDialog::OnRemoveScript);
	EventM("add_script", this, &WorldPropertiesDialog::OnAddScript);
	EventM("max_script_vars", this, &WorldPropertiesDialog::OnMaxScriptVars);
	EventMX("script_vars", "hui:change", this, &WorldPropertiesDialog::OnScriptVarEdit);
	//EventM("model_script_var_template", this, &ModelPropertiesDialog::OnModelScriptVarTemplate);

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
	SetFloat("sun_ang_x", ed->multi_view_3d->cam.ang.x * 180.0f / pi);
	SetFloat("sun_ang_y", ed->multi_view_3d->cam.ang.y * 180.0f / pi);
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
		temp.ScriptFile.add(ed->DialogFileComplete.substr(Script::config.Directory.num, -1));
		FillScriptList();
	}
}



void WorldPropertiesDialog::OnRemoveScript()
{
	int n = GetInt("script_list");
	if (n >= 0){
		temp.ScriptFile.erase(n);
		FillScriptList();
	}
}



void WorldPropertiesDialog::OnFogModeNone()
{
	Enable("fog_start", false);
	Enable("fog_end", false);
	Enable("fog_density", false);
	Enable("fog_color", false);
}

void WorldPropertiesDialog::OnFogModeLinear()
{
	Enable("fog_start", true);
	Enable("fog_end", true);
	Enable("fog_density", false);
	Enable("fog_color", true);
}

void WorldPropertiesDialog::OnFogModeExp()
{
	Enable("fog_start", false);
	Enable("fog_end", false);
	Enable("fog_density", true);
	Enable("fog_color", true);
}



void WorldPropertiesDialog::FillSkyboxList()
{
	HuiComboBoxSeparator = ":";
	Reset("skybox");
	foreachi(string &sb, temp.SkyBoxFile, i)
		AddString("skybox", format("%d:%s", i, sb.c_str()));
	HuiComboBoxSeparator = "\\";
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
	foreachi(float v, temp.ScriptVar, i)
		/*if (i<NumObjectScriptVarNames)
			AddString("script_vars", format("%d\\%s\\%.6f", i, ObjectScriptVarName[i].c_str(), v));
		else*/
			AddString("script_vars", format("%d\\\\%.6f", i, v));
}



void WorldPropertiesDialog::FillScriptList()
{
	HuiComboBoxSeparator = ":";
	Reset("script_list");
	foreach(string &s, temp.ScriptFile)
		AddString("script_list", s);
	Enable("remove_script", false);
	HuiComboBoxSeparator = "\\";
}



void WorldPropertiesDialog::ApplyData()
{
	temp.PhysicsEnabled = IsChecked("physics_enabled");
	temp.Gravity.x = GetFloat("gravitation_x");
	temp.Gravity.y = GetFloat("gravitation_y");
	temp.Gravity.z = GetFloat("gravitation_z");
	temp.BackGroundColor = GetColor("bgc");
	temp.FogEnabled = !IsChecked("fog_mode:none");
	if (IsChecked("fog_mode:linear"))
		temp.FogMode = FogLinear;
	else if (IsChecked("fog_mode:exp"))
		temp.FogMode = FogExp;
	else if (IsChecked("fog_mode:exp2"))
		temp.FogMode = FogExp2;
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
	if (temp.FogEnabled){
		if (temp.FogMode == FogLinear)
			Check("fog_mode:linear", true);
		else if (temp.FogMode == FogExp)
			Check("fog_mode:exp", true);
		else if (temp.FogMode == FogExp2)
			Check("fog_mode:exp2", true);
	}else{
		Check("fog_mode:none", true);
	}
	SetFloat("fog_start", temp.FogStart);
	SetFloat("fog_end", temp.FogEnd);
	SetFloat("fog_density", temp.FogDensity);
	SetColor("fog_color", temp.FogColor);
	Enable("fog_start", temp.FogEnabled && (temp.FogMode == FogLinear));
	Enable("fog_end", temp.FogEnabled && (temp.FogMode == FogLinear));
	Enable("fog_density", temp.FogEnabled && ((temp.FogMode == FogExp) || (temp.FogMode == FogExp2)));
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


