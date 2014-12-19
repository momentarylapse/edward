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
#include "../../../MultiView/MultiView.h"
#include "../../../lib/script/script.h"
#include "../../../lib/nix/nix.h"

#define WorldPhysicsDec			3
#define WorldLightDec			1
#define WorldFogDec				6

WorldPropertiesDialog::WorldPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	HuiWindow("world_dialog", _parent, _allow_parent),
	Observer("WorldPropertiesDialog")
{
	data = _data;

	setTooltip("bgc", _("Farbe des Himmels"));
	setTooltip("skybox", _("Modelle, die &uber die Hintergrundfarge gemalt werden\n- Doppelklick um ein Modell zu w&ahlen"));

	setTooltip("fog_start", _("Abstand, ab dem der Nebel beginnt (Intensit&at 0)"));
	setTooltip("fog_end", _("maximale Sichtweite, dahinter hat der Nebel volle Intensit&at"));
	setTooltip("fog_distance", _("Nebelintensit&at = exp( - Entfernung / Sichtweite )"));

	setTooltip("ambient", _("Generelles Umgebungslicht zus&atzlich zu allen Lichtquellen"));
	setTooltip("sun_am", _("ungerichtetes Umgebungslicht"));
	setTooltip("sun_di", _("frontales Licht"));
	setTooltip("sun_sp", _("Glanzlicht"));
	setTooltip("sun_ang_x", _("H&ohe &uber dem Horizont"));
	setTooltip("sun_ang_y", _("Kompassrichtung entlang des Horizonts"));

	event("cancel", this, &WorldPropertiesDialog::OnClose);
	event("hui:close", this, &WorldPropertiesDialog::OnClose);
	event("apply", this, &WorldPropertiesDialog::ApplyData);
	event("ok", this, &WorldPropertiesDialog::OnOk);

	event("sun_enabled", this, &WorldPropertiesDialog::OnSunEnabled);
	event("sun_ang_from_camera", this, &WorldPropertiesDialog::OnSunAngFromCamera);
	event("fog_mode:none", this, &WorldPropertiesDialog::OnFogModeNone);
	event("fog_mode:linear", this, &WorldPropertiesDialog::OnFogModeLinear);
	event("fog_mode:exp", this, &WorldPropertiesDialog::OnFogModeExp);
	event("fog_mode:exp2", this, &WorldPropertiesDialog::OnFogModeExp);
	event("skybox", this, &WorldPropertiesDialog::OnSkybox);
	eventX("skybox", "hui:select", this, &WorldPropertiesDialog::OnSkyboxSelect);
	event("remove_skybox", this, &WorldPropertiesDialog::OnRemoveSkybox);
	event("physics_enabled", this, &WorldPropertiesDialog::OnPhysicsEnabled);
	eventX("script_list", "hui:select", this, &WorldPropertiesDialog::OnScriptSelect);
	event("remove_script", this, &WorldPropertiesDialog::OnRemoveScript);
	event("add_script", this, &WorldPropertiesDialog::OnAddScript);
	event("max_script_vars", this, &WorldPropertiesDialog::OnMaxScriptVars);
	eventX("script_vars", "hui:change", this, &WorldPropertiesDialog::OnScriptVarEdit);
	//eventM("model_script_var_template", this, &ModelPropertiesDialog::OnModelScriptVarTemplate);

	subscribe(data);

	temp = data->meta_data;
	LoadData();
}

WorldPropertiesDialog::~WorldPropertiesDialog()
{
	mode_world->WorldDialog = NULL;
	unsubscribe(data);
}

void WorldPropertiesDialog::OnSkybox()
{
	int n = getInt("");
	if (ed->fileDialog(FD_MODEL,false,true)){
		temp.SkyBoxFile[n] = ed->dialog_file_no_ending;
		FillSkyboxList();
	}
}

void WorldPropertiesDialog::OnSkyboxSelect()
{
	int row = getInt("");
	if (row >= 0)
		enable("remove_skybox", temp.SkyBoxFile[row].num > 0);
	else
		enable("remove_skybox", false);
}


void WorldPropertiesDialog::OnScriptSelect()
{
	int row = getInt("");
	enable("remove_script", row >= 0);
}



void WorldPropertiesDialog::OnClose()
{
	delete(this);
}


void WorldPropertiesDialog::OnSunEnabled()
{
	bool b = isChecked("");
	enable("sun_am", b);
	enable("sun_di", b);
	enable("sun_sp", b);
	enable("sun_ang_x", b);
	enable("sun_ang_y", b);
	enable("sun_ang_from_camera", b);
}


void WorldPropertiesDialog::OnSunAngFromCamera()
{
	setFloat("sun_ang_x", ed->multi_view_3d->cam.ang.x * 180.0f / pi);
	setFloat("sun_ang_y", ed->multi_view_3d->cam.ang.y * 180.0f / pi);
}


void WorldPropertiesDialog::OnPhysicsEnabled()
{
	bool b = isChecked("");
	enable("gravitation_x", b);
	enable("gravitation_y", b);
	enable("gravitation_z", b);
}



void WorldPropertiesDialog::OnRemoveSkybox()
{
	int n = getInt("skybox");
	if (n >= 0)
		if (temp.SkyBoxFile[n].num > 0){
			temp.SkyBoxFile[n] = "";
			FillSkyboxList();
		}
}



void WorldPropertiesDialog::OnAddScript()
{
	if (ed->fileDialog(FD_SCRIPT, false, true)){
		temp.ScriptFile.add(ed->dialog_file_complete.substr(Script::config.Directory.num, -1));
		FillScriptList();
	}
}



void WorldPropertiesDialog::OnRemoveScript()
{
	int n = getInt("script_list");
	if (n >= 0){
		temp.ScriptFile.erase(n);
		FillScriptList();
	}
}



void WorldPropertiesDialog::OnFogModeNone()
{
	enable("fog_start", false);
	enable("fog_end", false);
	enable("fog_distance", false);
	enable("fog_color", false);
}

void WorldPropertiesDialog::OnFogModeLinear()
{
	enable("fog_start", true);
	enable("fog_end", true);
	enable("fog_distance", false);
	enable("fog_color", true);
}

void WorldPropertiesDialog::OnFogModeExp()
{
	enable("fog_start", false);
	enable("fog_end", false);
	enable("fog_distance", true);
	enable("fog_color", true);
}



void WorldPropertiesDialog::FillSkyboxList()
{
	HuiComboBoxSeparator = ":";
	reset("skybox");
	foreachi(string &sb, temp.SkyBoxFile, i)
		addString("skybox", format("%d:%s", i, sb.c_str()));
	HuiComboBoxSeparator = "\\";
	enable("remove_skybox", false);
}



void WorldPropertiesDialog::onUpdate(Observable *o, const string &message)
{
	temp = data->meta_data;
	LoadData();
}



void WorldPropertiesDialog::OnScriptVarEdit()
{
	int row = HuiGetEvent()->row;
	temp.ScriptVar[row] = s2f(getCell("script_vars", row, 2));
}



void WorldPropertiesDialog::FillScriptVarList()
{
	reset("script_vars");
	foreachi(float v, temp.ScriptVar, i)
		/*if (i<NumObjectScriptVarNames)
			addString("script_vars", format("%d\\%s\\%.6f", i, ObjectScriptVarName[i].c_str(), v));
		else*/
			addString("script_vars", format("%d\\\\%.6f", i, v));
}



void WorldPropertiesDialog::FillScriptList()
{
	HuiComboBoxSeparator = ":";
	reset("script_list");
	foreach(string &s, temp.ScriptFile)
		addString("script_list", s);
	enable("remove_script", false);
	HuiComboBoxSeparator = "\\";
}



void WorldPropertiesDialog::ApplyData()
{
	temp.PhysicsEnabled = isChecked("physics_enabled");
	temp.Gravity.x = getFloat("gravitation_x");
	temp.Gravity.y = getFloat("gravitation_y");
	temp.Gravity.z = getFloat("gravitation_z");
	temp.BackGroundColor = getColor("bgc");
	temp.FogEnabled = !isChecked("fog_mode:none");
	if (isChecked("fog_mode:linear"))
		temp.FogMode = FogLinear;
	else if (isChecked("fog_mode:exp"))
		temp.FogMode = FogExp;
	else if (isChecked("fog_mode:exp2"))
		temp.FogMode = FogExp2;
	temp.FogStart = getFloat("fog_start");
	temp.FogEnd = getFloat("fog_end");
	temp.FogDensity = 1.0f / getFloat("fog_distance");
	temp.FogColor = getColor("fog_color");

	temp.SunEnabled = isChecked("sun_enabled");
	temp.SunAmbient = getColor("sun_am");
	temp.SunDiffuse = getColor("sun_di");
	temp.SunSpecular = getColor("sun_sp");
	temp.SunAng.x = getFloat("sun_ang_x") / 180.0f * pi;
	temp.SunAng.y = getFloat("sun_ang_y") / 180.0f * pi;
	temp.Ambient = getColor("ambient");

	data->execute(new ActionWorldEditData(temp));
}



void WorldPropertiesDialog::OnScriptVarTemplate()
{
}



void WorldPropertiesDialog::OnMaxScriptVars()
{
	temp.ScriptVar.resize(getInt("max_script_vars"));
	FillScriptVarList();
}



void WorldPropertiesDialog::OnOk()
{
	ApplyData();
	delete(this);
}



void WorldPropertiesDialog::LoadData()
{
	setDecimals(WorldFogDec);
	setColor("bgc", temp.BackGroundColor);
	if (temp.FogEnabled){
		if (temp.FogMode == FogLinear)
			check("fog_mode:linear", true);
		else if (temp.FogMode == FogExp)
			check("fog_mode:exp", true);
		else if (temp.FogMode == FogExp2)
			check("fog_mode:exp2", true);
	}else{
		check("fog_mode:none", true);
	}
	setFloat("fog_start", temp.FogStart);
	setFloat("fog_end", temp.FogEnd);
	setFloat("fog_distance", 1.0f / temp.FogDensity);
	setColor("fog_color", temp.FogColor);
	enable("fog_start", temp.FogEnabled && (temp.FogMode == FogLinear));
	enable("fog_end", temp.FogEnabled && (temp.FogMode == FogLinear));
	enable("fog_distance", temp.FogEnabled && ((temp.FogMode == FogExp) || (temp.FogMode == FogExp2)));
	enable("fog_color", temp.FogEnabled);

	setDecimals(WorldLightDec);
	check("sun_enabled", temp.SunEnabled);
	setColor("sun_am", temp.SunAmbient);
	setColor("sun_di", temp.SunDiffuse);
	setColor("sun_sp", temp.SunSpecular);
	setFloat("sun_ang_x", temp.SunAng.x * 180.0f / pi);
	setFloat("sun_ang_y", temp.SunAng.y * 180.0f / pi);
	enable("sun_am", temp.SunEnabled);
	enable("sun_di", temp.SunEnabled);
	enable("sun_sp", temp.SunEnabled);
	enable("sun_ang_x", temp.SunEnabled);
	enable("sun_ang_y", temp.SunEnabled);
	enable("sun_ang_from_camera", temp.SunEnabled);
	setColor("ambient", temp.Ambient);

	setDecimals(WorldPhysicsDec);
	check("physics_enabled", temp.PhysicsEnabled);
	enable("gravitation_x", temp.PhysicsEnabled);
	enable("gravitation_y", temp.PhysicsEnabled);
	enable("gravitation_z", temp.PhysicsEnabled);
	setFloat("gravitation_x", temp.Gravity.x);
	setFloat("gravitation_y", temp.Gravity.y);
	setFloat("gravitation_z", temp.Gravity.z);

	setInt("max_script_vars", temp.ScriptVar.num);

	FillSkyboxList();
	FillScriptVarList();
	FillScriptList();
}


