/*
 * ModelMaterialDialog.cpp
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#include "ModelMaterialDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../Mesh/ModeModelMesh.h"
#include "../../../Action/Model/Data/ActionModelAddMaterial.h"
#include "../../../Action/Model/Data/ActionModelDeleteMaterial.h"
#include "../../../Action/Model/Data/ActionModelEditMaterial.h"
#include "../../../Action/Model/Data/ActionModelEditData.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
#include "../../../y/EngineData.h"
#include "../Mesh/ModeModelMeshTexture.h"

string file_secure(const Path &filename);
string render_material(ModelMaterial *m);

namespace nix {
	enum class Alpha;
}

ModelMaterialDialog::ModelMaterialDialog(DataModel *_data, bool full) {
	from_resource("model_material_dialog");

	data = _data;


	data->subscribe(this, [=]{
		if (apply_queue_depth == 0)
			load_data();
	}, data->MESSAGE_MATERIAL_CHANGE);
	data->subscribe(this, [=]{
		if (apply_queue_depth == 0)
			load_data();
	}, data->MESSAGE_TEXTURE_CHANGE);

	mode_model_mesh->state.subscribe(this, [=]{ load_data(); }, mode_model_mesh->state.MESSAGE_CURRENT_MATERIAL_CHANGE);
	mode_model_mesh_texture->state.subscribe(this, [=]{ load_data(); }, mode_model_mesh_texture->state.MESSAGE_TEXTURE_LEVEL_CHANGE);

	popup_materials = hui::CreateResourceMenu("model-material-list-popup");
	popup_textures = hui::CreateResourceMenu("model-texture-list-popup");

	event_x("material_list", "hui:select", [=]{ on_material_list_select(); });
	event_x("material_list", "hui:right-button-down", [=]{ on_material_list_right_click(); });
	event("add_new_material", [=]{ on_material_add(); });
	event("add_material", [=]{ on_material_load(); });
	event("delete_material", [=]{ on_material_delete(); });
	event("apply_material", [=]{ on_material_apply(); });


	event("texture-level-add", [=]{ on_texture_level_add(); });
	event("mat_textures", [=]{ on_textures(); });
	event_x("mat_textures", "hui:select", [=]{ on_textures_select(); });
	event_x("mat_textures", "hui:right-button-down", [=]{ on_textures_right_click(); });
	event("texture-level-delete", [=]{ on_texture_level_delete(); });
	event("texture-level-clear", [=]{ on_texture_level_clear(); });
	event("texture-level-load", [=]{ on_texture_level_load(); });
	event("texture-level-save", [=]{ on_texture_level_save(); });
	event("texture-level-scale", [=]{ on_texture_level_scale(); });
	event("transparency_mode:material", [=]{ on_transparency_mode(); });
	event("transparency_mode:none", [=]{ on_transparency_mode(); });
	event("transparency_mode:function", [=]{ on_transparency_mode(); });
	event("transparency_mode:color_key", [=]{ on_transparency_mode(); });
	event("transparency_mode:factor", [=]{ on_transparency_mode(); });

	event("override-colors", [=]{ on_override_colors(); });
	event("albedo", [=]{ apply_data_color(); });
	event("roughness", [=]{ apply_data_color(); });
	event("slider-roughness", [=]{ apply_data_color(); });
	event("metal", [=]{ apply_data_color(); });
	event("slider-metal", [=]{ apply_data_color(); });
	event("emission", [=]{ apply_data_color(); });

	event("alpha_factor", [=]{ apply_data_alpha(); });
	event("alpha_source", [=]{ apply_data_alpha(); });
	event("alpha_dest", [=]{ apply_data_alpha(); });
	event("alpha_z_buffer", [=]{ apply_data_alpha(); });

	hide_control("model_material_dialog_grp_color", !full);
	hide_control("model_material_dialog_grp_transparency", !full);

	expand_all("model_material_dialog_grp_textures", true);

	load_data();
	apply_queue_depth = 0;
}

ModelMaterialDialog::~ModelMaterialDialog() {
	mode_model_mesh_texture->state.unsubscribe(this);
	mode_model_mesh->state.unsubscribe(this);
	data->unsubscribe(this);

	delete popup_materials;
	delete popup_textures;
}

// data -> GUI
void ModelMaterialDialog::load_data() {
	auto col = data->material[mode_model_mesh->current_material]->col;
	auto alpha = data->material[mode_model_mesh->current_material]->alpha;
	fill_material_list();

	fill_texture_list();
	check("override-colors", col.user);
	enable("albedo", col.user);
	enable("roughness", col.user);
	enable("slider-roughness", col.user);
	enable("metal", col.user);
	enable("slider-metal", col.user);
	enable("emission", col.user);
	set_color("albedo", col.albedo);
	set_float("roughness", col.roughness);
	set_float("slider-roughness", col.roughness);
	set_float("metal", col.metal);
	set_float("slider-metal", col.metal);
	set_color("emission", col.emission);

	if (alpha.mode == TransparencyMode::COLOR_KEY_SMOOTH)
		check("transparency_mode:color_key", true);
	else if (alpha.mode == TransparencyMode::COLOR_KEY_HARD)
		check("transparency_mode:color_key", true);
	else if (alpha.mode == TransparencyMode::FACTOR)
		check("transparency_mode:factor", true);
	else if (alpha.mode == TransparencyMode::FUNCTIONS)
		check("transparency_mode:function", true);
	else if (alpha.mode == TransparencyMode::NONE)
		check("transparency_mode:none", true);
	else
		check("transparency_mode:material", true);
	enable("alpha_factor", alpha.mode == TransparencyMode::FACTOR);
	enable("alpha_source", alpha.mode == TransparencyMode::FUNCTIONS);
	enable("alpha_dest", alpha.mode == TransparencyMode::FUNCTIONS);
	set_float("alpha_factor", alpha.factor * 100.0f);
	check("alpha_z_buffer", alpha.zbuffer);
	set_int("alpha_source", (int)alpha.source);
	set_int("alpha_dest", (int)alpha.destination);
}


// GUI -> data
void ModelMaterialDialog::apply_data_color() {
	apply_queue_depth ++;

	auto col = data->material[mode_model_mesh->current_material]->col;
	auto parent = data->material[mode_model_mesh->current_material]->material;

	col.user= is_checked("override-colors");

	if (col.user) {
		col.albedo = get_color("albedo");
		col.roughness = get_float("slider-roughness");
		col.metal = get_float("slider-metal");
		col.emission = get_color("emission");
	} else {
		col.albedo = parent->albedo;
		col.roughness = parent->roughness;
		col.metal = parent->metal;
		col.emission = parent->emission;
	}
	enable("albedo", col.user);
	enable("roughness", col.user);
	enable("slider-roughness", col.user);
	enable("metal", col.user);
	enable("slider-metal", col.user);
	enable("emission", col.user);

	data->execute(new ActionModelEditMaterial(mode_model_mesh->current_material, col));
	apply_queue_depth --;
}

void ModelMaterialDialog::apply_data_alpha() {
	apply_queue_depth ++;
	auto alpha = data->material[mode_model_mesh->current_material]->alpha;

	if (is_checked("transparency_mode:function"))
		alpha.mode = TransparencyMode::FUNCTIONS;
	else if (is_checked("transparency_mode:color_key"))
		alpha.mode = TransparencyMode::COLOR_KEY_HARD;
	else if (is_checked("transparency_mode:factor"))
		alpha.mode = TransparencyMode::FACTOR;
	else if (is_checked("transparency_mode:none"))
		alpha.mode = TransparencyMode::NONE;
	else
		alpha.mode = TransparencyMode::DEFAULT;

	enable("alpha_factor", alpha.mode == TransparencyMode::FACTOR);
	enable("alpha_source", alpha.mode == TransparencyMode::FUNCTIONS);
	enable("alpha_dest", alpha.mode == TransparencyMode::FUNCTIONS);

	alpha.zbuffer = is_checked("alpha_z_buffer");
	alpha.factor = get_float("alpha_factor") * 0.01f;
	alpha.source = (nix::Alpha)get_int("alpha_source");
	alpha.destination = (nix::Alpha)get_int("alpha_dest");

	data->execute(new ActionModelEditMaterial(mode_model_mesh->current_material, alpha));
	apply_queue_depth --;
}

int count_material_polygons(DataModel *data, int index) {
	int n = 0;
	for (auto &t: data->mesh->polygon)
		if (t.material == index)
			n ++;
	return n;
}

void ModelMaterialDialog::fill_material_list() {
	reset("material_list");
	for (int i=0;i<data->material.num;i++) {
		int nt = count_material_polygons(data, i);
		string im = render_material(data->material[i]);
		add_string("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im, file_secure(data->material[i]->filename)));
	}
	set_int("material_list", mode_model_mesh->current_material);
}



void ModelMaterialDialog::on_material_list_select() {
	mode_model_mesh->set_current_material(get_int(""));
}

void ModelMaterialDialog::on_material_add() {
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialDialog::on_material_load() {
	if (storage->file_dialog(FD_MATERIAL, false, true))
		data->execute(new ActionModelAddMaterial(storage->dialog_file_no_ending));
}

void ModelMaterialDialog::on_material_delete() {
	if (count_material_polygons(data, mode_model_mesh->current_material) > 0) {
		ed->error_box(_("can only delete materials that are not applied to any polygons"));
		return;
	}
	data->execute(new ActionModelDeleteMaterial(mode_model_mesh->current_material));
}

void ModelMaterialDialog::on_material_apply() {
	data->setMaterialSelection(mode_model_mesh->current_material);
}


void ModelMaterialDialog::on_override_colors() {
	apply_data_color();
}

void ModelMaterialDialog::fill_texture_list() {
	auto mat = data->material[mode_model_mesh->current_material];
	reset("mat_textures");
	for (int i=0;i<mat->texture_levels.num;i++) {
		string id = format("image:material[%d]-texture[%d]", mode_model_mesh->current_material, i);
		auto *img = mat->texture_levels[i]->image;
		auto *icon = mat->texture_levels[i]->image->scale(48, 48);
		hui::SetImage(icon, id);
		string ext = format(" (%dx%d)", img->width, img->height);
		if (mat->texture_levels[i]->edited)
			ext += " *";
		add_string("mat_textures", format("Tex[%d]\\%s\\%s", i, id, (file_secure(mat->texture_levels[i]->filename) + ext)));
		delete icon;
	}
	set_int("mat_textures", mode_model_mesh_texture->current_texture_level);
}


void ModelMaterialDialog::on_texture_level_add() {
	auto temp = data->material[mode_model_mesh->current_material];
	if (temp->texture_levels.num >= MATERIAL_MAX_TEXTURES) {
		ed->error_box(format(_("Only %d texture levels allowed!"), MATERIAL_MAX_TEXTURES));
		return;
	}


	data->execute(new ActionModelMaterialAddTexture(mode_model_mesh->current_material));
}

void ModelMaterialDialog::on_textures() {
	on_textures_select();
	on_texture_level_load();
}

void ModelMaterialDialog::on_texture_level_load() {
	int sel = get_int("mat_textures");
	if (sel >= 0)
		if (storage->file_dialog(FD_TEXTURE, false, true))
			data->execute(new ActionModelMaterialLoadTexture(mode_model_mesh->current_material, sel, storage->dialog_file));
}

void ModelMaterialDialog::on_texture_level_save() {
	int sel = get_int("mat_textures");
	if (sel >= 0)
		if (storage->file_dialog(FD_TEXTURE, true, true)) {
			auto tl = data->material[mode_model_mesh->current_material]->texture_levels[sel];
			tl->image->save(engine.texture_dir << storage->dialog_file);
			tl->filename = storage->dialog_file; // ...
			tl->edited = false;
		}
}

class TextureScaleDialog : public hui::Dialog {
public:
	TextureScaleDialog(hui::Window *parent, int w, int h) : hui::Dialog(_("Scale texture"), 300, 100, parent, false) {
		width = height = -1;
		from_resource("texture-scale-dialog");
		set_int("width", w);
		set_int("height", h);
		event("ok", [=]{ on_ok(); });
		event("cancel", [=]{ on_cancel(); });
	}
	void on_ok() {
		width = get_int("width");
		height = get_int("height");
		request_destroy();
	}
	void on_cancel() {
		request_destroy();
	}
	int width, height;

	static bool ask(hui::Window *parent, int &w, int &h) {
		auto *dlg = new TextureScaleDialog(parent, w, h);
		dlg->run();
		w = dlg->width;
		h = dlg->height;
		delete dlg;
		return w > 0;
	}
};

void ModelMaterialDialog::on_texture_level_scale() {
	int sel = get_int("mat_textures");
	if (sel >= 0) {
		auto tl = data->material[mode_model_mesh->current_material]->texture_levels[sel];
		int w = tl->image->width, h = tl->image->height;
		if (TextureScaleDialog::ask(win, w, h))
			data->execute(new ActionModelMaterialScaleTexture(mode_model_mesh->current_material, sel, w, h));
	}
}

void ModelMaterialDialog::on_textures_select() {
	int sel = get_int("");
	mode_model_mesh_texture->set_current_texture_level(sel);
}

void ModelMaterialDialog::on_texture_level_delete() {
	int sel = get_int("mat_textures");
	if (sel >= 0) {
		if (data->material[mode_model_mesh->current_material]->texture_levels.num <= 1) {
			ed->error_box(_("At least one texture level has to exist!"));
			return;
		}
		data->execute(new ActionModelMaterialDeleteTexture(mode_model_mesh->current_material, sel));
	}
}

void ModelMaterialDialog::on_texture_level_clear() {
	int sel = get_int("mat_textures");
	if (sel >= 0)
		data->execute(new ActionModelMaterialLoadTexture(mode_model_mesh->current_material, sel, ""));
}

void ModelMaterialDialog::on_transparency_mode() {
	apply_data_alpha();
}

void ModelMaterialDialog::on_material_list_right_click() {
	int n = hui::GetEvent()->row;
	if (n >= 0) {
		mode_model_mesh->set_current_material(n);
	}
	popup_materials->enable("apply_material", n>=0);
	popup_materials->enable("delete_material", n>=0);
	popup_materials->open_popup(this);
}

void ModelMaterialDialog::on_textures_right_click() {
	int n = hui::GetEvent()->row;
	if (n >= 0) {
		mode_model_mesh_texture->set_current_texture_level(n);
	}
	popup_textures->enable("texture-level-delete", n>=0);
	popup_textures->enable("texture-level-clear", n>=0);
	popup_textures->enable("texture-level-load", n>=0);
	popup_textures->enable("texture-level-save", n>=0);
	popup_textures->enable("texture-level-scale", n>=0);
	popup_textures->open_popup(this);
}
