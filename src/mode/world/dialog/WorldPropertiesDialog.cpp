/*
 * WorldPropertiesDialog.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "WorldPropertiesDialog.h"
#include "ScriptVarsDialog.h"
#include "../ModeWorld.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include "../../../action/world/ActionWorldEditData.h"
#include "../../../multiview/MultiView.h"
#include "../../../lib/os/file.h"
#include "../../../lib/kaba/kaba.h"
#include "../../../lib/nix/nix.h"
#include "../../../y/World.h"

#define WorldPhysicsDec			3
#define WorldLightDec			1
#define WorldFogDec				6

WorldPropertiesDialog::WorldPropertiesDialog(DataWorld *_data) :
	obs::Node<hui::Dialog>("world_dialog", 400, 300, _data->session->win, true)
{
	from_resource("world_dialog");
	data = _data;
	active = true;

	event("cancel", [this] { on_close(); });
	event("hui:close", [this] { on_close(); });
	event("apply", [this] { apply_data(); });
	event("ok", [this] { on_ok(); });

	event("fog_mode:none", [this] { on_fog_mode_none(); });
	event("fog_mode:linear", [this] { on_fog_mode_linear(); });
	event("fog_mode:exp", [this] { on_fog_mode_exp(); });
	event("fog_mode:exp2", [this] { on_fog_mode_exp(); });
	event_x("skybox", "hui:activate", [this] { on_skybox_select(); });
	event_x("skybox", "hui:right-button-down", [this] { on_skybox_right_click(); });
	event_x("skybox", "hui:move", [this] { on_skybox_move(); });
	event("skybox-add", [this] { on_skybox_add(); });
	event("skybox-remove", [this] { on_skybox_remove(); });
	event("skybox-select", [this] { on_skybox_select(); });
	event("physics_enabled", [this] { on_physics_enabled(); });
	event_x("script_list", "hui:right-button-down", [this] { on_script_right_click(); });
	event_x("script_list", "hui:activate", [this] { on_edit_script_vars(); });
	event("remove_script", [this] { on_script_remove(); });
	event("add_script", [this] { on_script_add(); });
	event("create_script", [this] { on_create_script(); });
	event("edit_script_vars", [this] { on_edit_script_vars(); });
	event("edit_script", [this] { on_edit_script(); });

	popup_skybox = hui::create_resource_menu("world-skybox-popup", this);

	data->out_changed >> create_sink([this] {
		temp = data->meta_data;
		load_data();
	});

	temp = data->meta_data;
	load_data();
}

WorldPropertiesDialog::~WorldPropertiesDialog() {
	data->unsubscribe(this);
}

void WorldPropertiesDialog::on_skybox_move() {
	temp.skybox_files.move(hui::get_event()->row, hui::get_event()->row_target);
	fill_skybox_list();
}

void WorldPropertiesDialog::on_skybox_right_click() {
	int n = hui::get_event()->row;
	popup_skybox->enable("skybox-select", n >= 0);
	popup_skybox->enable("skybox-remove", n >= 0);
	popup_skybox->open_popup(this);
}

void WorldPropertiesDialog::on_skybox_add() {
	data->session->storage->file_dialog(FD_MODEL,false,true).on([this] (const auto& p) {
		temp.skybox_files.add(p.simple);
		fill_skybox_list();
	});
}

void WorldPropertiesDialog::on_skybox_select() {
	int n = get_int("skybox");
	data->session->storage->file_dialog(FD_MODEL,false,true).on([this, n] (const auto& p) {
		temp.skybox_files[n] = p.simple;
		fill_skybox_list();
	});
}


void WorldPropertiesDialog::on_script_right_click() {
	popup_script = hui::create_resource_menu("world-script-popup", this);
	int n = hui::get_event()->row;
	popup_script->enable("remove_script", n >= 0);
	popup_script->enable("edit_script_vars", n >= 0);
	popup_script->open_popup(this);
}



void WorldPropertiesDialog::on_close() {
	data->unsubscribe(this);
	hide();
	active = false;
}


void WorldPropertiesDialog::on_physics_enabled() {
	bool b = is_checked("");
	enable("physics-mode", b);
	enable("gravitation_x", b);
	enable("gravitation_y", b);
	enable("gravitation_z", b);
}



void WorldPropertiesDialog::on_skybox_remove() {
	int n = get_int("skybox");
	if (n >= 0) {
		temp.skybox_files.erase(n);
		fill_skybox_list();
	}
}



void WorldPropertiesDialog::on_script_add() {
	data->session->storage->file_dialog(FD_SCRIPT, false, true).on([this] (const auto& p) {
		WorldScript s;
		s.filename = p.complete.relative_to(kaba::config.directory);
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
	});
}



void WorldPropertiesDialog::on_script_remove() {
	int n = get_int("script_list");
	if (n >= 0) {
		temp.scripts.erase(n);
		fill_script_list();
	}
}

shared<const kaba::Class> get_class_by_base(shared<kaba::Module> s, const string &parent);



shared<const kaba::Class> get_class(shared<kaba::Module> s, const string &name) {
	for (auto t: s->tree->base_class->classes)
		if (t->name == name)
			return t;
	throw Exception(format(_("script does not contain a class named'%s'"), name));
	return nullptr;
}

void update_script_data(Session *session, ScriptInstanceData &s, const string &class_base_name, bool guess_class) {
	try {
		auto context = ownify(kaba::Context::create());
		auto ss = context->load_module(s.filename, true);

		shared<const kaba::Class> t;
		if (guess_class) {
			s.class_name = "";
			t = get_class_by_base(ss, "*." + class_base_name);
			s.class_name = t->cname(t->owner->base_class);
		} else {
			t = get_class(ss, s.class_name);
		}

		Array<string> wanted;
		auto tt = t;
		while (tt) {
			for (auto c: tt->constants)
				if (c->name == "PARAMETERS" and c->type == kaba::TypeString) {
					wanted = c->as_string().lower().replace("_", "").replace("\n", "").explode(",");
					break;
				}
			tt = tt->parent;
		}

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
	} catch(Exception &e) {
		session->error(e.message());
	}

}

void WorldPropertiesDialog::on_edit_script_vars() {
	int n = get_int("script_list");
	if (n >= 0) {
		update_script_data(data->session, temp.scripts[n], "Controller", true);
		hui::fly(new ScriptVarsDialog(this, &temp.scripts[n]));
	}
}

void WorldPropertiesDialog::on_edit_script() {
	int n = get_int("script_list");
	if (n >= 0) {
		auto filename = kaba::config.directory | temp.scripts[n].filename;
		//int r = system(format("sgribthmaker '%s'", filename).c_str());
		hui::open_document(filename);
	}
}

void WorldPropertiesDialog::on_create_script() {
	data->session->storage->file_dialog(FD_SCRIPT, true, true).on([this] (const auto& p) {
		string source = R""""(use y

class X extends Controller
	let PARAMETERS = ""

	func override on_init()
		pass

	func override on_delete()
		pass

	func override on_iterate(dt: float)
		pass

	func override on_input()
		pass

	func override on_left_button_down()
		pass

	func override on_key_down(k: Key)
		pass
)"""";
		os::fs::write_text(p.complete, source);

		WorldScript s;
		s.filename = p.complete.relative_to(kaba::config.directory);
		temp.scripts.add(s);
		fill_script_list();
	});
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
	hui::separator = ":";
	reset("skybox");
	foreachi(auto &sb, temp.skybox_files, i)
		add_string("skybox", format("%d:%s", i, sb));
	hui::separator = "\\";
}





void WorldPropertiesDialog::fill_script_list() {
	hui::separator = ":";
	reset("script_list");
	for (auto &s: temp.scripts)
		add_string("script_list", s.filename.str());
	enable("remove_script", false);
	enable("edit_script_vars", false);
	hui::separator = "\\";
}



void WorldPropertiesDialog::apply_data() {
	temp.physics_enabled = is_checked("physics_enabled");
	if (get_int("physics-mode") == 1)
		temp.physics_mode = PhysicsMode::FULL_EXTERNAL;
	else
		temp.physics_mode = PhysicsMode::SIMPLE;
	temp.gravity.x = get_float("gravitation_x");
	temp.gravity.y = get_float("gravitation_y");
	temp.gravity.z = get_float("gravitation_z");
	temp.background_color = get_color("bgc");
	temp.fog.enabled = !is_checked("fog_mode:none");
	if (is_checked("fog_mode:linear"))
		temp.fog.mode = nix::FogMode::LINEAR;
	else if (is_checked("fog_mode:exp"))
		temp.fog.mode = nix::FogMode::EXP;
	else if (is_checked("fog_mode:exp2"))
		temp.fog.mode = nix::FogMode::EXP2;
	temp.fog.start = get_float("fog_start");
	temp.fog.end = get_float("fog_end");
	temp.fog.density = 1.0f / get_float("fog_distance");
	temp.fog.col = get_color("fog_color");

	data->execute(new ActionWorldEditData(temp));
}



void WorldPropertiesDialog::on_ok() {
	apply_data();
	on_close();
}

void WorldPropertiesDialog::restart() {
	data->out_changed >> create_sink([=] {
		temp = data->meta_data;
		load_data();
	});

	temp = data->meta_data;
	load_data();
	active = true;
}



void WorldPropertiesDialog::load_data() {
	set_decimals(WorldFogDec);
	set_color("bgc", temp.background_color);
	if (temp.fog.enabled) {
		if (temp.fog.mode == nix::FogMode::LINEAR)
			check("fog_mode:linear", true);
		else if (temp.fog.mode == nix::FogMode::EXP)
			check("fog_mode:exp", true);
		else if (temp.fog.mode == nix::FogMode::EXP2)
			check("fog_mode:exp2", true);
	} else {
		check("fog_mode:none", true);
	}
	set_float("fog_start", temp.fog.start);
	set_float("fog_end", temp.fog.end);
	set_float("fog_distance", 1.0f / temp.fog.density);
	set_color("fog_color", temp.fog.col);
	enable("fog_start", temp.fog.enabled and (temp.fog.mode == nix::FogMode::LINEAR));
	enable("fog_end", temp.fog.enabled and (temp.fog.mode == nix::FogMode::LINEAR));
	enable("fog_distance", temp.fog.enabled and ((temp.fog.mode == nix::FogMode::EXP) or (temp.fog.mode == nix::FogMode::EXP2)));
	enable("fog_color", temp.fog.enabled);

	set_decimals(WorldPhysicsDec);
	check("physics_enabled", temp.physics_enabled);
	if (temp.physics_mode == PhysicsMode::FULL_EXTERNAL)
		set_int("physics-mode", 1);
	else
		set_int("physics-mode", 0);
	enable("gravitation_x", temp.physics_enabled);
	enable("gravitation_y", temp.physics_enabled);
	enable("gravitation_z", temp.physics_enabled);
	set_float("gravitation_x", temp.gravity.x);
	set_float("gravitation_y", temp.gravity.y);
	set_float("gravitation_z", temp.gravity.z);

	fill_skybox_list();
	fill_script_list();
}


