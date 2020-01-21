/*
 * WorldPropertiesDialog.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "WorldPropertiesDialog.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
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

	popup_skybox = hui::CreateResourceMenu("world-skybox-popup");
	popup_script = hui::CreateResourceMenu("world-script-popup");

	event("cancel", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
	event("apply", [=]{ apply_data(); });
	event("ok", [=]{ on_ok(); });

	event("sun_enabled", [=]{ on_sun_enabled(); });
	event("sun_ang_from_camera", [=]{ on_sun_ang_from_camera(); });
	event("fog_mode:none", [=]{ on_fog_mode_none(); });
	event("fog_mode:linear", [=]{ on_fog_mode_linear(); });
	event("fog_mode:exp", [=]{ on_fog_mode_exp(); });
	event("fog_mode:exp2", [=]{ on_fog_mode_exp(); });
	event_x("skybox", "hui:activate", [=]{ on_skybox_select(); });
	event_x("skybox", "hui:right-button-down", [=]{ on_skybox_right_click(); });
	event("skybox-remove", [=]{ on_skybox_remove(); });
	event("skybox-select", [=]{ on_skybox_select(); });
	event("physics_enabled", [=]{ on_physics_enabled(); });
	event_x("script_list", "hui:right-button-down", [=]{ on_script_right_click(); });
	event_x("script_list", "hui:activate", [=]{ on_edit_script_vars(); });
	event("remove_script", [=]{ on_script_remove(); });
	event("add_script", [=]{ on_script_add(); });
	event("edit_script_vars", [=]{ on_edit_script_vars(); });

	subscribe(data);

	temp = data->meta_data;
	load_data();
}

WorldPropertiesDialog::~WorldPropertiesDialog() {
	mode_world->WorldDialog = NULL;
	unsubscribe(data);
	delete popup_skybox;
	delete popup_script;
}

void WorldPropertiesDialog::on_skybox_right_click() {
	int n = hui::GetEvent()->row;
	popup_skybox->enable("skybox-select", n >= 0);
	popup_skybox->enable("skybox-remove", n >= 0);
	popup_skybox->open_popup(this);
}

void WorldPropertiesDialog::on_skybox_select() {
	int n = get_int("skybox");
	if (storage->file_dialog(FD_MODEL,false,true)) {
		temp.SkyBoxFile[n] = storage->dialog_file_no_ending;
		fill_skybox_list();
	}
}


void WorldPropertiesDialog::on_script_right_click() {
	int n = hui::GetEvent()->row;
	popup_script->enable("remove_script", n >= 0);
	popup_script->enable("edit_script_vars", n >= 0);
	popup_script->open_popup(this);
}



void WorldPropertiesDialog::on_close() {
	unsubscribe(data);
	hide();
	active = false;
}


void WorldPropertiesDialog::on_sun_enabled() {
	bool b = is_checked("");
	enable("sun_am", b);
	enable("sun_di", b);
	enable("sun_sp", b);
	enable("sun_ang_x", b);
	enable("sun_ang_y", b);
	enable("sun_ang_from_camera", b);
}


void WorldPropertiesDialog::on_sun_ang_from_camera() {
	set_float("sun_ang_x", ed->multi_view_3d->cam.ang.x * 180.0f / pi);
	set_float("sun_ang_y", ed->multi_view_3d->cam.ang.y * 180.0f / pi);
}


void WorldPropertiesDialog::on_physics_enabled() {
	bool b = is_checked("");
	enable("gravitation_x", b);
	enable("gravitation_y", b);
	enable("gravitation_z", b);
}



void WorldPropertiesDialog::on_skybox_remove() {
	int n = get_int("skybox");
	if (n >= 0)
		if (temp.SkyBoxFile[n].num > 0) {
			temp.SkyBoxFile[n] = "";
			fill_skybox_list();
		}
}



void WorldPropertiesDialog::on_script_add() {
	if (storage->file_dialog(FD_SCRIPT, false, true)) {
		WorldScript s;
		s.filename = storage->dialog_file_complete.substr(Kaba::config.directory.num, -1);
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
		fill_script_list();
	}
}



void WorldPropertiesDialog::on_script_remove() {
	int n = get_int("script_list");
	if (n >= 0) {
		temp.scripts.erase(n);
		fill_script_list();
	}
}

void update_script_data(WorldScript &s) {
	s.class_name = "";
	try {
		auto ss = Kaba::Load(s.filename, true);

		Array<string> wanted;
		for (auto c: ss->syntax->base_class->constants)
			if (c->name == "PARAMETERS" and c->type == Kaba::TypeString)
				wanted = c->as_string().lower().replace("_", "").replace("\n", "").explode(",");

		for (auto *t: ss->syntax->base_class->classes) {
			if (!t->is_derived_from_s("Controller"))
				continue;
			s.class_name = t->name;
			for (auto &e: t->elements) {
				string nn = e.name.replace("_", "").lower();
				if (!sa_contains(wanted, nn))
					continue;
				bool found = false;
				for (auto &v: s.variables)
					if (v.name.lower().replace("_", "") == nn) {
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
	} catch(Exception &e) {

	}

}

void WorldPropertiesDialog::on_edit_script_vars() {
	int n = get_int("script_list");
	if (n >= 0) {
		update_script_data(temp.scripts[n]);
		auto dlg = new ScriptVarsDialog(this, &temp.scripts[n]);
		dlg->run();
		delete dlg;
	}
}



void WorldPropertiesDialog::on_fog_mode_none() {
	enable("fog_start", false);
	enable("fog_end", false);
	enable("fog_distance", false);
	enable("fog_color", false);
}

void WorldPropertiesDialog::on_fog_mode_linear() {
	enable("fog_start", true);
	enable("fog_end", true);
	enable("fog_distance", false);
	enable("fog_color", true);
}

void WorldPropertiesDialog::on_fog_mode_exp() {
	enable("fog_start", false);
	enable("fog_end", false);
	enable("fog_distance", true);
	enable("fog_color", true);
}



void WorldPropertiesDialog::fill_skybox_list() {
	hui::ComboBoxSeparator = ":";
	reset("skybox");
	foreachi(string &sb, temp.SkyBoxFile, i)
		add_string("skybox", format("%d:%s", i, sb.c_str()));
	hui::ComboBoxSeparator = "\\";
}



void WorldPropertiesDialog::on_update(Observable *o, const string &message) {
	temp = data->meta_data;
	load_data();
}



void WorldPropertiesDialog::fill_script_list() {
	hui::ComboBoxSeparator = ":";
	reset("script_list");
	for (auto &s: temp.scripts)
		add_string("script_list", s.filename);
	enable("remove_script", false);
	enable("edit_script_vars", false);
	hui::ComboBoxSeparator = "\\";
}



void WorldPropertiesDialog::apply_data() {
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



void WorldPropertiesDialog::on_ok() {
	apply_data();
	on_close();
}

void WorldPropertiesDialog::restart() {
	subscribe(data);

	temp = data->meta_data;
	load_data();
	active = true;
}



void WorldPropertiesDialog::load_data() {
	set_decimals(WorldFogDec);
	set_color("bgc", temp.BackGroundColor);
	if (temp.FogEnabled) {
		if (temp.FogMode == FOG_LINEAR)
			check("fog_mode:linear", true);
		else if (temp.FogMode == FOG_EXP)
			check("fog_mode:exp", true);
		else if (temp.FogMode == FOG_EXP2)
			check("fog_mode:exp2", true);
	} else {
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

	fill_skybox_list();
	fill_script_list();
}


