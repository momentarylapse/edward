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
#include "ScriptVarsDialog.h"

#define WorldPhysicsDec			3
#define WorldLightDec			1
#define WorldFogDec				6

WorldPropertiesDialog::WorldPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data) :
	hui::Dialog("world_dialog", 400, 300, _parent, _allow_parent),
	Observer("WorldPropertiesDialog")
{
	from_resource("world_dialog");
	data = _data;
	active = true;

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
	event_x("skybox", "hui:select", std::bind(&WorldPropertiesDialog::OnSkyboxSelect, this));
	event("remove_skybox", std::bind(&WorldPropertiesDialog::OnRemoveSkybox, this));
	event("physics_enabled", std::bind(&WorldPropertiesDialog::OnPhysicsEnabled, this));
	event_x("script_list", "hui:select", std::bind(&WorldPropertiesDialog::OnScriptSelect, this));
	event_x("script_list", "hui:activate", std::bind(&WorldPropertiesDialog::OnEditScriptVars, this));
	event("remove_script", std::bind(&WorldPropertiesDialog::OnRemoveScript, this));
	event("add_script", std::bind(&WorldPropertiesDialog::OnAddScript, this));
	event("edit_script_vars", std::bind(&WorldPropertiesDialog::OnEditScriptVars, this));

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
	int n = get_int("");
	if (ed->file_dialog(FD_MODEL,false,true)){
		temp.SkyBoxFile[n] = ed->dialog_file_no_ending;
		FillSkyboxList();
	}
}

void WorldPropertiesDialog::OnSkyboxSelect()
{
	int row = get_int("");
	if (row >= 0)
		enable("remove_skybox", temp.SkyBoxFile[row].num > 0);
	else
		enable("remove_skybox", false);
}


void WorldPropertiesDialog::OnScriptSelect()
{
	int row = get_int("");
	enable("remove_script", row >= 0);
	enable("edit_script_vars", row >= 0);
}



void WorldPropertiesDialog::OnClose()
{
	unsubscribe(data);
	hide();
	active = false;
}


void WorldPropertiesDialog::OnSunEnabled()
{
	bool b = is_checked("");
	enable("sun_am", b);
	enable("sun_di", b);
	enable("sun_sp", b);
	enable("sun_ang_x", b);
	enable("sun_ang_y", b);
	enable("sun_ang_from_camera", b);
}


void WorldPropertiesDialog::OnSunAngFromCamera()
{
	set_float("sun_ang_x", ed->multi_view_3d->cam.ang.x * 180.0f / pi);
	set_float("sun_ang_y", ed->multi_view_3d->cam.ang.y * 180.0f / pi);
}


void WorldPropertiesDialog::OnPhysicsEnabled()
{
	bool b = is_checked("");
	enable("gravitation_x", b);
	enable("gravitation_y", b);
	enable("gravitation_z", b);
}



void WorldPropertiesDialog::OnRemoveSkybox()
{
	int n = get_int("skybox");
	if (n >= 0)
		if (temp.SkyBoxFile[n].num > 0){
			temp.SkyBoxFile[n] = "";
			FillSkyboxList();
		}
}



void WorldPropertiesDialog::OnAddScript()
{
	if (ed->file_dialog(FD_SCRIPT, false, true)){
		WorldScript s;
		s.filename = ed->dialog_file_complete.substr(Kaba::config.directory.num, -1);
		temp.scripts.add(s);
		/*try{
			auto ss = Kaba::Load(s.filename, true);

			Array<string> wanted;
			for (auto c:ss->syntax->constants)
				if (c->name == "PARAMETERS" and c->type == Kaba::TypeString)
					wanted = c->as_string().lower().replace("_", "").replace("\n", "").explode(",");

			for (auto *t: ss->syntax->classes)
				if (t->is_derived_from("Controller")){
					for (auto &e: t->elements){
						if (sa_contains(wanted, e.name.replace("_", "").lower()))
							msg_write("    > " + e.name);
					}
				}
		}catch(Exception &e){

		}*/
		FillScriptList();
	}
}



void WorldPropertiesDialog::OnRemoveScript()
{
	int n = get_int("script_list");
	if (n >= 0){
		temp.scripts.erase(n);
		FillScriptList();
	}
}

void update_script_data(WorldScript &s)
{
	s.class_name = "";
	try{
		auto ss = Kaba::Load(s.filename, true);

		Array<string> wanted;
		for (auto c: ss->syntax->base_class->constants)
			if (c->name == "PARAMETERS" and c->type == Kaba::TypeString)
				wanted = c->as_string().lower().replace("_", "").replace("\n", "").explode(",");

		for (auto *t: ss->syntax->base_class->classes){
			if (!t->is_derived_from_s("Controller"))
				continue;
			s.class_name = t->name;
			for (auto &e: t->elements){
				string nn = e.name.replace("_", "").lower();
				if (!sa_contains(wanted, nn))
					continue;
				bool found = false;
				for (auto &v: s.variables)
					if (v.name.lower().replace("_", "") == nn){
						v.name = e.name;
						v.type = e.type->name;
						found = true;
					}
				if (found)
					continue;

				WorldScriptVariable v;
				v.name = e.name;
				v.type = e.type->name;
				s.variables.add(v);
			}
		}
	}catch(Exception &e){

	}

}

void WorldPropertiesDialog::OnEditScriptVars()
{
	int n = get_int("script_list");
	if (n >= 0){
		update_script_data(temp.scripts[n]);
		auto dlg = new ScriptVarsDialog(this, &temp.scripts[n]);
		dlg->run();
		delete dlg;
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
		add_string("skybox", format("%d:%s", i, sb.c_str()));
	hui::ComboBoxSeparator = "\\";
	enable("remove_skybox", false);
}



void WorldPropertiesDialog::on_update(Observable *o, const string &message)
{
	temp = data->meta_data;
	LoadData();
}



void WorldPropertiesDialog::FillScriptList()
{
	hui::ComboBoxSeparator = ":";
	reset("script_list");
	for (auto &s: temp.scripts)
		add_string("script_list", s.filename);
	enable("remove_script", false);
	enable("edit_script_vars", false);
	hui::ComboBoxSeparator = "\\";
}



void WorldPropertiesDialog::ApplyData()
{
	temp.PhysicsEnabled = is_checked("physics_enabled");
	temp.Gravity.x = get_float("gravitation_x");
	temp.Gravity.y = get_float("gravitation_y");
	temp.Gravity.z = get_float("gravitation_z");
	temp.BackGroundColor = get_color("bgc");
	temp.FogEnabled = !is_checked("fog_mode:none");
	if (is_checked("fog_mode:linear"))
		temp.FogMode = FOG_LINEAR;
	else if (is_checked("fog_mode:exp"))
		temp.FogMode = FOG_EXP;
	else if (is_checked("fog_mode:exp2"))
		temp.FogMode = FOG_EXP2;
	temp.FogStart = get_float("fog_start");
	temp.FogEnd = get_float("fog_end");
	temp.FogDensity = 1.0f / get_float("fog_distance");
	temp.FogColor = get_color("fog_color");

	temp.SunEnabled = is_checked("sun_enabled");
	temp.SunAmbient = get_color("sun_am");
	temp.SunDiffuse = get_color("sun_di");
	temp.SunSpecular = get_color("sun_sp");
	temp.SunAng.x = get_float("sun_ang_x") / 180.0f * pi;
	temp.SunAng.y = get_float("sun_ang_y") / 180.0f * pi;
	temp.Ambient = get_color("ambient");

	data->execute(new ActionWorldEditData(temp));
}



void WorldPropertiesDialog::OnOk()
{
	ApplyData();
	OnClose();
}

void WorldPropertiesDialog::restart()
{
	subscribe(data);

	temp = data->meta_data;
	LoadData();
	active = true;
}



void WorldPropertiesDialog::LoadData()
{
	set_decimals(WorldFogDec);
	set_color("bgc", temp.BackGroundColor);
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
	set_float("fog_start", temp.FogStart);
	set_float("fog_end", temp.FogEnd);
	set_float("fog_distance", 1.0f / temp.FogDensity);
	set_color("fog_color", temp.FogColor);
	enable("fog_start", temp.FogEnabled and (temp.FogMode == FOG_LINEAR));
	enable("fog_end", temp.FogEnabled and (temp.FogMode == FOG_LINEAR));
	enable("fog_distance", temp.FogEnabled and ((temp.FogMode == FOG_EXP) or (temp.FogMode == FOG_EXP2)));
	enable("fog_color", temp.FogEnabled);

	set_decimals(WorldLightDec);
	check("sun_enabled", temp.SunEnabled);
	set_color("sun_am", temp.SunAmbient);
	set_color("sun_di", temp.SunDiffuse);
	set_color("sun_sp", temp.SunSpecular);
	set_float("sun_ang_x", temp.SunAng.x * 180.0f / pi);
	set_float("sun_ang_y", temp.SunAng.y * 180.0f / pi);
	enable("sun_am", temp.SunEnabled);
	enable("sun_di", temp.SunEnabled);
	enable("sun_sp", temp.SunEnabled);
	enable("sun_ang_x", temp.SunEnabled);
	enable("sun_ang_y", temp.SunEnabled);
	enable("sun_ang_from_camera", temp.SunEnabled);
	set_color("ambient", temp.Ambient);

	set_decimals(WorldPhysicsDec);
	check("physics_enabled", temp.PhysicsEnabled);
	enable("gravitation_x", temp.PhysicsEnabled);
	enable("gravitation_y", temp.PhysicsEnabled);
	enable("gravitation_z", temp.PhysicsEnabled);
	set_float("gravitation_x", temp.Gravity.x);
	set_float("gravitation_y", temp.Gravity.y);
	set_float("gravitation_z", temp.Gravity.z);

	FillSkyboxList();
	FillScriptList();
}


