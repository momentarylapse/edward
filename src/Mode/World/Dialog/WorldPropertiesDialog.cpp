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
#include "../../../lib/kaba/kaba.h"
#include "../../../lib/nix/nix.h"

#define WorldPhysicsDec			3
#define WorldLightDec			1
#define WorldFogDec				6

WorldPropertiesDialog::WorldPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data) :
	hui::Dialog("world_dialog", 400, 300, _parent, _allow_parent),
	Observer("WorldPropertiesDialog")
{
	fromResource("world_dialog");
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

	event("cancel", std::bind(&WorldPropertiesDialog::OnClose, this));
	event("hui:close", std::bind(&WorldPropertiesDialog::OnClose, this));
	event("apply", std::bind(&WorldPropertiesDialog::ApplyData, this));
	event("ok", std::bind(&WorldPropertiesDialog::OnOk, this));

	event("sun_enabled", std::bind(&WorldPropertiesDialog::OnSunEnabled, this));
	event("sun_ang_from_camera", std::bind(&WorldPropertiesDialog::OnSunAngFromCamera, this));
	event("fog_mode:none", std::bind(&WorldPropertiesDialog::OnFogModeNone, this));
	event("fog_mode:linear", std::bind(&WorldPropertiesDialog::OnFogModeLinear, this));
	event("fog_mode:exp", std::bind(&WorldPropertiesDialog::OnFogModeExp, this));
	event("fog_mode:exp2", std::bind(&WorldPropertiesDialog::OnFogModeExp, this));
	event("skybox", std::bind(&WorldPropertiesDialog::OnSkybox, this));
	eventX("skybox", "hui:select", std::bind(&WorldPropertiesDialog::OnSkyboxSelect, this));
	event("remove_skybox", std::bind(&WorldPropertiesDialog::OnRemoveSkybox, this));
	event("physics_enabled", std::bind(&WorldPropertiesDialog::OnPhysicsEnabled, this));
	eventX("script_list", "hui:select", std::bind(&WorldPropertiesDialog::OnScriptSelect, this));
	event("remove_script", std::bind(&WorldPropertiesDialog::OnRemoveScript, this));
	event("add_script", std::bind(&WorldPropertiesDialog::OnAddScript, this));
	event("max_script_vars", std::bind(&WorldPropertiesDialog::OnMaxScriptVars, this));
	eventX("script_vars", "hui:change", std::bind(&WorldPropertiesDialog::OnScriptVarEdit, this));
	//eventM("model_script_var_template", std::bind(&ModelPropertiesDialog::OnModelScriptVarTemplate, this));

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
	destroy();
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
		temp.ScriptFile.add(ed->dialog_file_complete.substr(Kaba::config.directory.num, -1));
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
	hui::ComboBoxSeparator = ":";
	reset("skybox");
	foreachi(string &sb, temp.SkyBoxFile, i)
		addString("skybox", format("%d:%s", i, sb.c_str()));
	hui::ComboBoxSeparator = "\\";
	enable("remove_skybox", false);
}



void WorldPropertiesDialog::onUpdate(Observable *o, const string &message)
{
	temp = data->meta_data;
	LoadData();
}



void WorldPropertiesDialog::OnScriptVarEdit()
{
	int row = hui::GetEvent()->row;
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
	hui::ComboBoxSeparator = ":";
	reset("script_list");
	for (string &s: temp.ScriptFile)
		addString("script_list", s);
	enable("remove_script", false);
	hui::ComboBoxSeparator = "\\";
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
		temp.FogMode = FOG_LINEAR;
	else if (isChecked("fog_mode:exp"))
		temp.FogMode = FOG_EXP;
	else if (isChecked("fog_mode:exp2"))
		temp.FogMode = FOG_EXP2;
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
		if (temp.FogMode == FOG_LINEAR)
			check("fog_mode:linear", true);
		else if (temp.FogMode == FOG_EXP)
			check("fog_mode:exp", true);
		else if (temp.FogMode == FOG_EXP2)
			check("fog_mode:exp2", true);
	}else{
		check("fog_mode:none", true);
	}
	setFloat("fog_start", temp.FogStart);
	setFloat("fog_end", temp.FogEnd);
	setFloat("fog_distance", 1.0f / temp.FogDensity);
	setColor("fog_color", temp.FogColor);
	enable("fog_start", temp.FogEnabled and (temp.FogMode == FOG_LINEAR));
	enable("fog_end", temp.FogEnabled and (temp.FogMode == FOG_LINEAR));
	enable("fog_distance", temp.FogEnabled and ((temp.FogMode == FOG_EXP) or (temp.FogMode == FOG_EXP2)));
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


