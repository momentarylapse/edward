/*
 * MaterialPropertiesDialog.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "MaterialPropertiesDialog.h"
#include "MaterialRenderPassDialog.h"
#include "../ModeMaterial.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include "../../../action/material/ActionMaterialEditAppearance.h"
#include "../../../action/material/ActionMaterialEditPhysics.h"
#include <lib/base/iter.h>
#include <lib/nix/nix.h>
#include <y/helper/ResourceManager.h>


string file_secure(const Path &filename); // -> ModelPropertiesDialog


MaterialPropertiesDialog::MaterialPropertiesDialog(hui::Window *_parent, DataMaterial *_data) {
	from_resource("material_dialog");
	data = _data;

	temp = data->appearance;
	temp_phys = data->physics;
	apply_queue_depth = 0;
	apply_phys_queue_depth = 0;

	textures_popup = hui::create_resource_menu("material-texture-list-popup", this);
	passes_popup = hui::create_resource_menu("material-render-pass-list-popup", this);

	// dialog
	event("textures", [this] { on_texture_level_select_file(); });
	event_x("textures", hui::EventID::SELECT, [this] { on_textures_select(); });
	event_x("textures", hui::EventID::RIGHT_BUTTON_DOWN, [this] { on_textures_right_click(); });
	event("texture-level-select-file", [this] { on_texture_level_select_file(); });
	event("texture-level-add", [this] { on_texture_level_add(); });
	event("texture-level-delete", [this] { on_texture_level_delete(); });
	event("texture-level-clear", [this] { on_texture_level_clear(); });

	event_x("passes", hui::EventID::RIGHT_BUTTON_DOWN, [this] { on_passes_right_click(); });
	event("render-pass-edit", [this] { on_pass_edit(); });
	event("render-pass-add", [this] { on_pass_add(); });
	event("render-pass-copy", [this] { on_pass_copy(); });
	event("render-pass-delete", [this] { on_pass_delete(); });


	event("albedo", [this] { apply_data(); });
	event("roughness", [this] {
		set_float("slider-roughness", get_float(""));
		apply_data_delayed();
	});
	event("slider-roughness", [this] {
		set_float("roughness", get_float(""));
		apply_data_delayed();
	});
	event("metal", [this] {
		set_float("slider-metal", get_float(""));
		apply_data_delayed();
	});
	event("slider-metal", [this] {
		set_float("metal", get_float(""));
		apply_data_delayed();
	});
	event("emission", [this] { apply_data(); });

	event("rcjump", [this] { apply_phys_data_delayed(); });
	event("rcstatic", [this] { apply_phys_data_delayed(); });
	event("rcsliding", [this] { apply_phys_data_delayed(); });
	event("rcroll", [this] { apply_phys_data_delayed(); });

	expand_row("grp-color", 0, true);
	expand_row("grp-textures", 0, true);
	expand_row("grp-passes", 0, true);

	set_options("shader_file", "placeholder=- engine default shader -");
	load_data();
	data->out_changed >> create_sink([this] { on_data_update(); });
}

void MaterialPropertiesDialog::load_data() {
	fill_texture_list();
	fill_passes_list();
	set_color("albedo", temp.albedo);
	set_float("roughness", temp.roughness);
	set_float("slider-roughness", temp.roughness);
	set_float("metal", temp.metal);
	set_float("slider-metal", temp.metal);
	set_color("emission", temp.emissive);

	set_float("rcjump", temp_phys.friction_jump);
	set_float("rcstatic", temp_phys.friction_static);
	set_float("rcsliding", temp_phys.friction_sliding);
	set_float("rcroll", temp_phys.friction_rolling);
}

MaterialPropertiesDialog::~MaterialPropertiesDialog() {
	data->unsubscribe(this);
}

void MaterialPropertiesDialog::on_data_update() {
	temp = data->appearance;
	temp_phys = data->physics;
	load_data();
}

void MaterialPropertiesDialog::on_texture_level_add() {
	if (temp.texture_files.num >= MATERIAL_MAX_TEXTURES){
		data->session->error(format(_("Only %d texture levels allowed!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.texture_files.add("");
	apply_data();
}

void MaterialPropertiesDialog::on_texture_level_select_file() {
	int sel = get_int("textures");
	if ((sel >= 0) and (sel < temp.texture_files.num))
		data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, sel] (const auto& p) {
			temp.texture_files[sel] = p.relative;
			apply_data();
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			fill_texture_list();
		});
}

void MaterialPropertiesDialog::on_textures_select() {
	int sel = get_int("textures");
	enable("mat_delete_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
	enable("mat_empty_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
}

void MaterialPropertiesDialog::on_textures_right_click() {
	int sel = get_int("textures");
	textures_popup->enable("texture-level-select-file", sel >= 0);
	textures_popup->enable("texture-level-clear", sel >= 0);
	textures_popup->enable("texture-level-delete", sel >= 0);
	textures_popup->open_popup(this);
}

void MaterialPropertiesDialog::on_texture_level_delete() {
	int sel = get_int("textures");
	if (sel >= 0) {
		temp.texture_files.erase(sel);
		apply_data();
		fill_texture_list();
	}
}

void MaterialPropertiesDialog::on_texture_level_clear() {
	int sel = get_int("textures");
	if (sel >= 0) {
		temp.texture_files[sel] = "";
		apply_data();
		fill_texture_list();
	}
}



void MaterialPropertiesDialog::on_passes_right_click() {
	int sel = get_int("");
	textures_popup->enable("render-pass-edit", sel >= 0);
	textures_popup->enable("render-pass-copy", sel >= 0);
	textures_popup->enable("render-pass-delete", sel >= 0);
	passes_popup->open_popup(this);
}

void MaterialPropertiesDialog::on_pass_edit() {
	int sel = get_int("passes");
	if (sel < 0)
		return;
	auto dlg = new MaterialRenderPassDialog(win, data->appearance.passes[sel]);
	hui::fly(dlg).then([this, dlg, sel] {
		if (dlg->success) {
			temp.passes[sel] = dlg->result;
			data->execute(new ActionMaterialEditAppearance(temp));
		}
	});
}

void MaterialPropertiesDialog::on_pass_add() {
	temp.passes.add({});
	data->execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::on_pass_copy() {
	int sel = get_int("passes");
	if (sel < 0)
		return;
	temp.passes.add(temp.passes[sel]);
	data->execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::on_pass_delete() {
	int sel = get_int("passes");
	if (sel < 0)
		return;
	if (data->appearance.passes.num <= 1) {
		data->session->error(_("At least 1 render pass is required"));
		return;
	}
	temp.passes.erase(sel);
	data->execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::apply_data() {
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	temp.albedo = get_color("albedo");
	temp.roughness = get_float("roughness");
	temp.metal = get_float("metal");
	temp.emissive = get_color("emission");

	data->execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::apply_data_delayed() {
	apply_queue_depth ++;
	hui::run_later(0.2f, [this] { apply_data(); });
}

void MaterialPropertiesDialog::apply_phys_data() {
	if (apply_phys_queue_depth> 0)
		apply_phys_queue_depth --;
	if (apply_phys_queue_depth > 0)
		return;
	temp_phys.friction_jump = get_float("rcjump");
	temp_phys.friction_static = get_float("rcstatic");
	temp_phys.friction_sliding = get_float("rcsliding");
	temp_phys.friction_rolling = get_float("rcroll");

	data->execute(new ActionMaterialEditPhysics(temp_phys));
}

void MaterialPropertiesDialog::apply_phys_data_delayed() {
	apply_phys_queue_depth ++;
	hui::run_later(0.2f, [this] { apply_phys_data(); });
}

void MaterialPropertiesDialog::fill_texture_list() {
	reset("textures");
	for (int i=0;i<temp.texture_files.num;i++) {
		auto tex = data->session->resource_manager->load_texture(temp.texture_files[i]);
		string img = data->session->get_tex_image(tex.get());
		add_string("textures", format("Tex[%d]\\%s\\%s", i, img, file_secure(temp.texture_files[i])));
	}
	if (temp.texture_files.num == 0)
		add_string("textures", _("\\\\   - no textures -"));
	enable("mat_delete_texture_level", false);
	enable("mat_empty_texture_level", false);
}

string transparency_to_str(TransparencyMode m) {
	if (m == TransparencyMode::NONE)
		return "solid";
	if (m == TransparencyMode::FUNCTIONS)
		return "transparent";
	if (m == TransparencyMode::MIX)
		return "transparent";
	if (m == TransparencyMode::COLOR_KEY_HARD)
		return "color keyed";
	return "???";
}

void MaterialPropertiesDialog::fill_passes_list() {
	reset("passes");
	for (auto&& [i,p]: enumerate(temp.passes)) {
		string desc;
		if (p.shader.file.is_empty())
			desc += "(default shader)";
		else
			desc += format("<b>%s</b>", p.shader.file);
		string sub = transparency_to_str(p.mode);
		if (p.culling == 2)
			sub += " back";

		desc += "\n<small>  " + sub + "</small>";
		add_string("passes", format("%d.\\%s", i+1, desc));
	}
}
