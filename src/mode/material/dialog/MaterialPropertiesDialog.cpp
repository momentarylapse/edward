/*
 * MaterialPropertiesDialog.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "MaterialPropertiesDialog.h"
#include "../ModeMaterial.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include "../../../action/material/ActionMaterialEditAppearance.h"
#include "../../../action/material/ActionMaterialEditPhysics.h"
#include "../../../lib/nix/nix.h"
#include <y/helper/ResourceManager.h>


string file_secure(const Path &filename); // -> ModelPropertiesDialog


MaterialPropertiesDialog::MaterialPropertiesDialog(hui::Window *_parent, DataMaterial *_data) {
	from_resource("material_dialog");
	data = _data;

	temp = data->appearance;
	temp_phys = data->physics;
	apply_queue_depth = 0;
	apply_phys_queue_depth = 0;

	// dialog
	event("mat_add_texture_level", [this]{ on_add_texture_level(); });
	event("mat_textures", [this]{ on_textures(); });
	event_x("mat_textures", "hui:select", [this]{ on_textures_select(); });
	event("mat_delete_texture_level", [this]{ on_delete_texture_level(); });
	event("mat_empty_texture_level", [this]{ on_clear_texture_level(); });
	event("transparency_mode:none", [this]{ on_transparency_mode(); });
	event("transparency_mode:function", [this]{ on_transparency_mode(); });
	event("transparency_mode:color_key", [this]{ on_transparency_mode(); });
	event("transparency_mode:factor", [this]{ on_transparency_mode(); });
	event("reflection_mode:none", [this]{ on_reflection_mode(); });
	event("reflection_mode:cube_static", [this]{ on_reflection_mode(); });
	event("reflection_mode:cube_dynamic", [this]{ on_reflection_mode(); });
	event("reflection_textures", [this]{ on_reflection_textures(); });


	event("albedo", [this]{ apply_data(); });
	event("roughness", [this]{
		set_float("slider-roughness", get_float(""));
		apply_data_delayed();
	});
	event("slider-roughness", [this]{
		set_float("roughness", get_float(""));
		apply_data_delayed();
	});
	event("metal", [this]{
		set_float("slider-metal", get_float(""));
		apply_data_delayed();
	});
	event("slider-metal", [this]{
		set_float("metal", get_float(""));
		apply_data_delayed();
	});
	event("emission", [this]{ apply_data(); });

	event("alpha_factor", [this]{ apply_data_delayed(); });
	event("alpha_source", [this]{ apply_data_delayed(); });
	event("alpha_dest", [this]{ apply_data_delayed(); });
	event("alpha_z_buffer", [this]{ apply_data(); });

	event("rcjump", [this]{ apply_phys_data_delayed(); });
	event("rcstatic", [this]{ apply_phys_data_delayed(); });
	event("rcsliding", [this]{ apply_phys_data_delayed(); });
	event("rcroll", [this]{ apply_phys_data_delayed(); });

	expand_row("material_dialog_grp_color", 0, true);

	set_options("shader_file", "placeholder=- engine default shader -");
	load_data();
	data->out_changed >> create_sink([this]{ on_data_update(); });
}

void MaterialPropertiesDialog::load_data() {
	fill_texture_list();
	set_color("albedo", temp.albedo);
	set_float("roughness", temp.roughness);
	set_float("slider-roughness", temp.roughness);
	set_float("metal", temp.metal);
	set_float("slider-metal", temp.metal);
	set_color("emission", temp.emissive);

	if (temp.transparency_mode == TransparencyMode::COLOR_KEY_SMOOTH)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TransparencyMode::COLOR_KEY_HARD)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TransparencyMode::FACTOR)
		check("transparency_mode:factor", true);
	else if (temp.transparency_mode == TransparencyMode::FUNCTIONS)
		check("transparency_mode:function", true);
	else
		check("transparency_mode:none", true);
	enable("alpha_factor", temp.transparency_mode == TransparencyMode::FACTOR);
	enable("alpha_source", temp.transparency_mode == TransparencyMode::FUNCTIONS);
	enable("alpha_dest", temp.transparency_mode == TransparencyMode::FUNCTIONS);
	set_float("alpha_factor", temp.alpha_factor * 100.0f);
	check("alpha_z_buffer", temp.alpha_z_buffer);
	set_int("alpha_source", (int)temp.alpha_source);
	set_int("alpha_dest", (int)temp.alpha_destination);

	if (temp.reflection_mode == ReflectionMode::CUBE_MAP_STATIC)
		check("reflection_mode:cube_static", true);
	else if (temp.reflection_mode == ReflectionMode::CUBE_MAP_DYNAMIC)
		check("reflection_mode:cube_dynamic", true);
	else
		check("reflection_mode:none", true);
	if (temp.reflection_size >= 512)
		set_int("reflection_size", 3);
	else if (temp.reflection_size >= 256)
		set_int("reflection_size", 2);
	else if (temp.reflection_size >= 128)
		set_int("reflection_size", 1);
	else
		set_int("reflection_size", 0);
	set_int("reflection_density", temp.reflection_density);
	refill_refl_tex_view();
	enable("reflection_size", ((temp.reflection_mode == ReflectionMode::CUBE_MAP_STATIC) or (temp.reflection_mode == ReflectionMode::CUBE_MAP_DYNAMIC)));
	enable("reflection_textures", (temp.reflection_mode == ReflectionMode::CUBE_MAP_STATIC));
	enable("reflection_density", (temp.reflection_mode != ReflectionMode::NONE));


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

void MaterialPropertiesDialog::on_add_texture_level() {
	if (temp.texture_files.num >= MATERIAL_MAX_TEXTURES){
		data->session->error(format(_("Only %d texture levels allowed!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.texture_files.add("");
	apply_data();
}

void MaterialPropertiesDialog::on_textures() {
	int sel = get_int("");
	if ((sel >= 0) and (sel < temp.texture_files.num))
		data->session->storage->file_dialog(FD_TEXTURE, false, true).on([this, sel] (const auto& p) {
			temp.texture_files[sel] = p.relative;
			apply_data();
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			fill_texture_list();
		});
}

void MaterialPropertiesDialog::on_textures_select() {
	int sel = get_int("");
	enable("mat_delete_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
	enable("mat_empty_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
}

void MaterialPropertiesDialog::on_delete_texture_level() {
	int sel = get_int("mat_textures");
	if (sel >= 0) {
		temp.texture_files.erase(sel);
		apply_data();
		fill_texture_list();
	}
}

void MaterialPropertiesDialog::on_clear_texture_level() {
	int sel = get_int("mat_textures");
	if (sel >= 0) {
		temp.texture_files[sel] = "";
		apply_data();
		fill_texture_list();
	}
}

void MaterialPropertiesDialog::on_transparency_mode() {
	if (is_checked("transparency_mode:function"))
		temp.transparency_mode = TransparencyMode::FUNCTIONS;
	else if (is_checked("transparency_mode:color_key"))
		temp.transparency_mode = TransparencyMode::COLOR_KEY_HARD;
	else if (is_checked("transparency_mode:factor"))
		temp.transparency_mode = TransparencyMode::FACTOR;
	else
		temp.transparency_mode = TransparencyMode::NONE;
	enable("alpha_factor", temp.transparency_mode == TransparencyMode::FACTOR);
	enable("alpha_source", temp.transparency_mode == TransparencyMode::FUNCTIONS);
	enable("alpha_dest", temp.transparency_mode == TransparencyMode::FUNCTIONS);
	apply_data();
}

void MaterialPropertiesDialog::on_reflection_mode() {
	if (is_checked("reflection_mode:cube_static"))
		temp.reflection_mode = ReflectionMode::CUBE_MAP_STATIC;
	else if (is_checked("reflection_mode:cube_dynamic"))
		temp.reflection_mode = ReflectionMode::CUBE_MAP_DYNAMIC;
	else
		temp.reflection_mode = ReflectionMode::NONE;
	enable("reflection_size", ((temp.reflection_mode == ReflectionMode::CUBE_MAP_STATIC) or (temp.reflection_mode == ReflectionMode::CUBE_MAP_DYNAMIC)));
	enable("reflection_textures", (temp.reflection_mode == ReflectionMode::CUBE_MAP_STATIC));
	enable("reflection_density", (temp.reflection_mode != ReflectionMode::NONE));
	apply_data();
}

void MaterialPropertiesDialog::on_reflection_textures() {
#if 0
	int sel=get_int("");
	if (storage->file_dialog(FD_TEXTURE,false,true)) {
		temp.reflection_texture_file[sel] = storage->dialog_file;
		if ((sel==0) and (temp.reflection_texture_file[0].find(".") >= 0)) {
			int p=temp.reflection_texture_file[0].find(".");
			for (int i=1;i<6;i++) {
				string tf;
				tf = temp.reflection_texture_file[0];
				tf[p-1]=temp.reflection_texture_file[0][p-1]+i;
				if (file_exists(nix::texture_dir + tf)) {
					temp.reflection_texture_file[i] = tf;
				}
			}

		}
		apply_data();
		refill_refl_tex_view();
	}
#endif
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
	temp.alpha_z_buffer = is_checked("alpha_z_buffer");
	temp.alpha_factor = get_float("alpha_factor") * 0.01f;
	temp.alpha_source = (nix::Alpha)get_int("alpha_source");
	temp.alpha_destination = (nix::Alpha)get_int("alpha_dest");

	temp.reflection_density = get_int("reflection_density");
	temp.reflection_size = 64 << get_int("reflection_size");

	data->execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::apply_data_delayed() {
	apply_queue_depth ++;
	hui::run_later(0.2f, [this]{ apply_data(); });
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
	hui::run_later(0.2f, [this]{ apply_phys_data(); });
}

void MaterialPropertiesDialog::refill_refl_tex_view() {
	reset("reflection_textures");
	add_string("reflection_textures", " + X\\" + temp.reflection_texture_file[0].str());
	add_string("reflection_textures", " - X\\" + temp.reflection_texture_file[1].str());
	add_string("reflection_textures", " + Y\\" + temp.reflection_texture_file[2].str());
	add_string("reflection_textures", " - Y\\" + temp.reflection_texture_file[3].str());
	add_string("reflection_textures", " + Z\\" + temp.reflection_texture_file[4].str());
	add_string("reflection_textures", " - Z\\" + temp.reflection_texture_file[5].str());
}

void MaterialPropertiesDialog::fill_texture_list() {
	reset("mat_textures");
	for (int i=0;i<temp.texture_files.num;i++) {
		auto tex = data->session->resource_manager->load_texture(temp.texture_files[i]);
		string img = data->session->get_tex_image(tex.get());
		add_string("mat_textures", format("Tex[%d]\\%s\\%s", i, img, file_secure(temp.texture_files[i])));
	}
	if (temp.texture_files.num == 0)
		add_string("mat_textures", _("\\\\   - no textures -"));
	enable("mat_delete_texture_level", false);
	enable("mat_empty_texture_level", false);
}
