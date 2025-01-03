/*
 * ModelMaterialDialog.cpp
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#include "ModelMaterialDialog.h"
#include "../mesh/ModeModelMesh.h"
#include "../mesh/ModeModelMeshTexture.h"
#include "../ModeModel.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../data/model/ModelPolygon.h"
#include "../../../data/model/ModelMaterial.h"
#include "../../../action/model/data/ActionModelAddMaterial.h"
#include "../../../action/model/data/ActionModelDeleteMaterial.h"
#include "../../../action/model/data/ActionModelEditMaterial.h"
#include "../../../action/model/data/ActionModelEditData.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include "../../../lib/image/image.h"
#include <y/EngineData.h>

string file_secure(const Path &filename);
string render_material(ModelMaterial *m);

namespace nix {
	enum class Alpha;
}

ModelMaterialDialog::ModelMaterialDialog(DataModel *_data, bool full) {
	from_resource("model_material_dialog");

	data = _data;


	data->out_material_changed >> create_sink([this] {
		if (apply_queue_depth == 0)
			load_data();
	});
	data->out_texture_changed >> create_sink([this] {
		if (apply_queue_depth == 0)
			load_data();
	});

	mode_model_mesh()->state.out_current_material_changed >> create_sink([this] { load_data(); });
	mode_model_mesh_texture()->state.out_texture_level_changed >> create_sink([this] { load_data(); });

	popup_materials = hui::create_resource_menu("model-material-list-popup", this);
	popup_textures = hui::create_resource_menu("model-texture-list-popup", this);

	event_x("material_list", "hui:select", [this] { on_material_list_select(); });
	event_x("material_list", "hui:right-button-down", [this] { on_material_list_right_click(); });
	event("add_new_material", [this] { on_material_add(); });
	event("add_material", [this] { on_material_load(); });
	event("delete_material", [this] { on_material_delete(); });
	event("apply_material", [this] { on_material_apply(); });


	event("texture-level-add", [this] { on_texture_level_add(); });
	event("mat_textures", [this] { on_textures(); });
	event_x("mat_textures", "hui:select", [this] { on_textures_select(); });
	event_x("mat_textures", "hui:right-button-down", [this] { on_textures_right_click(); });
	event("texture-level-delete", [this] { on_texture_level_delete(); });
	event("texture-level-clear", [this] { on_texture_level_clear(); });
	event("texture-level-load", [this] { on_texture_level_load(); });
	event("texture-level-save", [this] { on_texture_level_save(); });
	event("texture-level-scale", [this] { on_texture_level_scale(); });

	event("override-colors", [this] { on_override_colors(); });
	event("albedo", [this] { apply_data_color(); });
	event("roughness", [this] { apply_data_color(); });
	event("slider-roughness", [this] { apply_data_color(); });
	event("metal", [this] { apply_data_color(); });
	event("slider-metal", [this] { apply_data_color(); });
	event("emission", [this] { apply_data_color(); });

	hide_control("model_material_dialog_grp_color", !full);
	hide_control("model_material_dialog_grp_transparency", !full);

	expand("grp-color", true);
	expand("grp-textures", true);

	load_data();
	apply_queue_depth = 0;
}

ModelMaterialDialog::~ModelMaterialDialog() {
	mode_model_mesh_texture()->state.unsubscribe(this);
	mode_model_mesh()->state.unsubscribe(this);
	data->unsubscribe(this);

	delete popup_materials;
	delete popup_textures;
}


ModeModelMesh *ModelMaterialDialog::mode_model_mesh() {
	return data->session->mode_model->mode_model_mesh;
}
ModeModelMeshTexture *ModelMaterialDialog::mode_model_mesh_texture() {
	return mode_model_mesh()->mode_model_mesh_texture;
}

// data -> GUI
void ModelMaterialDialog::load_data() {
	auto col = data->material[mode_model_mesh()->current_material]->col;
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
}


// GUI -> data
void ModelMaterialDialog::apply_data_color() {
	apply_queue_depth ++;

	auto col = data->material[mode_model_mesh()->current_material]->col;
	auto parent = data->material[mode_model_mesh()->current_material]->material;

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
	set_float("metal", col.metal);
	set_float("roughness", col.roughness);
	enable("albedo", col.user);
	enable("roughness", col.user);
	enable("slider-roughness", col.user);
	enable("metal", col.user);
	enable("slider-metal", col.user);
	enable("emission", col.user);

	data->execute(new ActionModelEditMaterial(mode_model_mesh()->current_material, col));
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
	set_int("material_list", mode_model_mesh()->current_material);
}



void ModelMaterialDialog::on_material_list_select() {
	mode_model_mesh()->set_current_material(get_int(""));
}

void ModelMaterialDialog::on_material_add() {
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialDialog::on_material_load() {
	data->session->storage->file_dialog(FD_MATERIAL, false, true).then([this] (const auto& p){
		data->execute(new ActionModelAddMaterial(p.simple));
	});
}

void ModelMaterialDialog::on_material_delete() {
	if (count_material_polygons(data, mode_model_mesh()->current_material) > 0) {
		data->session->error(_("can only delete materials that are not applied to any polygons"));
		return;
	}
	data->execute(new ActionModelDeleteMaterial(mode_model_mesh()->current_material));
}

void ModelMaterialDialog::on_material_apply() {
	data->setMaterialSelection(mode_model_mesh()->current_material);
}


void ModelMaterialDialog::on_override_colors() {
	apply_data_color();
}

void ModelMaterialDialog::fill_texture_list() {
	auto mat = data->material[mode_model_mesh()->current_material];
	reset("mat_textures");
	for (int i=0;i<mat->texture_levels.num;i++) {
		string id = format("image:material[%d]-texture[%d]", mode_model_mesh()->current_material, i);
		auto *img = mat->texture_levels[i].image;
		auto *icon = mat->texture_levels[i].image->scale(48, 48);
		hui::set_image(icon, id);
		string ext = format(" (%dx%d)", img->width, img->height);
		if (mat->texture_levels[i].edited)
			ext += " *";
		add_string("mat_textures", format("Tex[%d]\\%s\\%s", i, id, (file_secure(mat->texture_levels[i].filename) + ext)));
		delete icon;
	}
	set_int("mat_textures", mode_model_mesh_texture()->current_texture_level);
}


void ModelMaterialDialog::on_texture_level_add() {
	auto temp = data->material[mode_model_mesh()->current_material];
	if (temp->texture_levels.num >= MATERIAL_MAX_TEXTURES) {
		data->session->error(format(_("Only %d texture levels allowed!"), MATERIAL_MAX_TEXTURES));
		return;
	}


	data->execute(new ActionModelMaterialAddTexture(mode_model_mesh()->current_material));
}

void ModelMaterialDialog::on_textures() {
	on_textures_select();
	on_texture_level_load();
}

void ModelMaterialDialog::on_texture_level_load() {
	int sel = get_int("mat_textures");
	if (sel >= 0)
		data->session->storage->file_dialog(FD_TEXTURE, false, true).then([this, sel] (const auto& p) {
			data->execute(new ActionModelMaterialLoadTexture(mode_model_mesh()->current_material, sel, p.relative));
		});
}

void ModelMaterialDialog::on_texture_level_save() {
	int sel = get_int("mat_textures");
	if (sel >= 0)
		data->session->storage->file_dialog(FD_TEXTURE, true, true).then([this, sel] (const auto& p) {
			auto& tl = data->material[mode_model_mesh()->current_material]->texture_levels[sel];
			tl.image->save(p.complete);
			tl.filename = p.relative; // ...
			tl.edited = false;
		});
}

class TextureScaleDialog : public hui::Dialog {
public:
	TextureScaleDialog(hui::Window *parent, int w, int h) : hui::Dialog(_("Scale texture"), 300, 100, parent, false) {
		width = height = -1;
		from_resource("texture-scale-dialog");
		set_int("width", w);
		set_int("height", h);
		event("ok", [this] { on_ok(); });
		event("cancel", [this] { on_cancel(); });
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

	static void ask(hui::Window *parent, int &w, int &h, std::function<void(int,int)> cb_success) {
		auto *dlg = new TextureScaleDialog(parent, w, h);
		hui::fly(dlg).then([dlg, cb_success] {
			if (dlg->width > 0)
				cb_success(dlg->width, dlg->height);
		});
	}
};

void ModelMaterialDialog::on_texture_level_scale() {
	int sel = get_int("mat_textures");
	if (sel >= 0) {
		auto& tl = data->material[mode_model_mesh()->current_material]->texture_levels[sel];
		int w = tl.image->width;
		int h = tl.image->height;
		TextureScaleDialog::ask(win, w, h, [this, sel] (int _w, int _h) {
			data->execute(new ActionModelMaterialScaleTexture(mode_model_mesh()->current_material, sel, _w, _h));
		});
	}
}

void ModelMaterialDialog::on_textures_select() {
	int sel = get_int("");
	mode_model_mesh_texture()->set_current_texture_level(sel);
}

void ModelMaterialDialog::on_texture_level_delete() {
	int sel = get_int("mat_textures");
	if (sel >= 0) {
		if (data->material[mode_model_mesh()->current_material]->texture_levels.num <= 1) {
			data->session->error(_("At least one texture level has to exist!"));
			return;
		}
		data->execute(new ActionModelMaterialDeleteTexture(mode_model_mesh()->current_material, sel));
	}
}

void ModelMaterialDialog::on_texture_level_clear() {
	int sel = get_int("mat_textures");
	if (sel >= 0)
		data->execute(new ActionModelMaterialLoadTexture(mode_model_mesh()->current_material, sel, ""));
}

void ModelMaterialDialog::on_material_list_right_click() {
	int n = hui::get_event()->row;
	if (n >= 0) {
		mode_model_mesh()->set_current_material(n);
	}
	popup_materials->enable("apply_material", n>=0);
	popup_materials->enable("delete_material", n>=0);
	popup_materials->open_popup(this);
}

void ModelMaterialDialog::on_textures_right_click() {
	int n = hui::get_event()->row;
	if (n >= 0) {
		mode_model_mesh_texture()->set_current_texture_level(n);
	}
	popup_textures->enable("texture-level-delete", n>=0);
	popup_textures->enable("texture-level-clear", n>=0);
	popup_textures->enable("texture-level-load", n>=0);
	popup_textures->enable("texture-level-save", n>=0);
	popup_textures->enable("texture-level-scale", n>=0);
	popup_textures->open_popup(this);
}
