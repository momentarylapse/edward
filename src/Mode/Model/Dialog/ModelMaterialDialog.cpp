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
#include "../../../Action/Model/Data/ActionModelEditMaterial.h"
#include "../../../Action/Model/Data/ActionModelEditData.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
#include "../../../lib/nix/nix.h"
#include "../Mesh/ModeModelMeshTexture.h"

string file_secure(const string &filename);
string render_material(ModelMaterial *m);

ModelMaterialDialog::ModelMaterialDialog(DataModel *_data, bool full) :
	Observer("ModelMaterialDialog") {
	from_resource("model_material_dialog");

	data = _data;
	subscribe(data);
	subscribe(mode_model_mesh, mode_model_mesh->MESSAGE_CURRENT_MATERIAL_CHANGE);
	subscribe(mode_model_mesh_texture, mode_model_mesh_texture->MESSAGE_TEXTURE_LEVEL_CHANGE);

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

	event("default_colors", [=]{ on_default_colors(); });
	event("mat_am", [=]{ apply_data_color(); });
	event("mat_di", [=]{ apply_data_color(); });
	event("mat_sp", [=]{ apply_data_color(); });
	event("mat_em", [=]{ apply_data_color(); });
	event("mat_shininess", [=]{ apply_data_color(); });

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
	unsubscribe(mode_model_mesh_texture);
	unsubscribe(mode_model_mesh);
	unsubscribe(data);

	delete popup_materials;
	delete popup_textures;
}

// data -> GUI
void ModelMaterialDialog::load_data() {
	auto col = data->material[mode_model_mesh->current_material]->col;
	auto alpha = data->material[mode_model_mesh->current_material]->alpha;
	fill_material_list();

	fill_texture_list();
	check("default_colors", !col.user);
	enable("mat_am", col.user);
	enable("mat_di", col.user);
	enable("mat_sp", col.user);
	enable("mat_em", col.user);
	enable("mat_shininess", col.user);
	set_color("mat_am", col.ambient);
	set_color("mat_di", col.diffuse);
	set_color("mat_sp", col.specular);
	set_color("mat_em", col.emission);
	set_float("mat_shininess", col.shininess);

	if (alpha.mode == TransparencyModeColorKeySmooth)
		check("transparency_mode:color_key", true);
	else if (alpha.mode == TransparencyModeColorKeyHard)
		check("transparency_mode:color_key", true);
	else if (alpha.mode == TransparencyModeFactor)
		check("transparency_mode:factor", true);
	else if (alpha.mode == TransparencyModeFunctions)
		check("transparency_mode:function", true);
	else if (alpha.mode == TransparencyModeNone)
		check("transparency_mode:none", true);
	else
		check("transparency_mode:material", true);
	enable("alpha_factor", alpha.mode == TransparencyModeFactor);
	enable("alpha_source", alpha.mode == TransparencyModeFunctions);
	enable("alpha_dest", alpha.mode == TransparencyModeFunctions);
	set_float("alpha_factor", alpha.factor * 100.0f);
	check("alpha_z_buffer", alpha.zbuffer);
	set_int("alpha_source", alpha.source);
	set_int("alpha_dest", alpha.destination);
}


// GUI -> data
void ModelMaterialDialog::apply_data_color() {
	apply_queue_depth ++;

	auto col = data->material[mode_model_mesh->current_material]->col;
	auto parent = data->material[mode_model_mesh->current_material]->material;

	col.user= !is_checked("default_colors");

	if (col.user) {
		col.ambient = get_color("mat_am");
		col.diffuse = get_color("mat_di");
		col.specular = get_color("mat_sp");
		col.emission = get_color("mat_em");
		col.shininess = get_float("mat_shininess");
	} else {
		col.ambient = parent->ambient;
		col.diffuse = parent->diffuse;
		col.specular = parent->specular;
		col.emission = parent->emission;
		col.shininess = parent->shininess;
	}
	enable("mat_am", col.user);
	enable("mat_di", col.user);
	enable("mat_sp", col.user);
	enable("mat_em", col.user);
	enable("mat_shininess", col.user);

	data->execute(new ActionModelEditMaterial(mode_model_mesh->current_material, col));
	apply_queue_depth --;
}

void ModelMaterialDialog::apply_data_alpha() {
	apply_queue_depth ++;
	auto alpha = data->material[mode_model_mesh->current_material]->alpha;

	if (is_checked("transparency_mode:function"))
		alpha.mode = TransparencyModeFunctions;
	else if (is_checked("transparency_mode:color_key"))
		alpha.mode = TransparencyModeColorKeyHard;
	else if (is_checked("transparency_mode:factor"))
		alpha.mode = TransparencyModeFactor;
	else if (is_checked("transparency_mode:none"))
		alpha.mode = TransparencyModeNone;
	else
		alpha.mode = TransparencyModeDefault;

	enable("alpha_factor", alpha.mode == TransparencyModeFactor);
	enable("alpha_source", alpha.mode == TransparencyModeFunctions);
	enable("alpha_dest", alpha.mode == TransparencyModeFunctions);

	alpha.zbuffer = is_checked("alpha_z_buffer");
	alpha.factor = get_float("alpha_factor") * 0.01f;
	alpha.source = get_int("alpha_source");
	alpha.destination = get_int("alpha_dest");

	data->execute(new ActionModelEditMaterial(mode_model_mesh->current_material, alpha));
	apply_queue_depth --;
}

void ModelMaterialDialog::fill_material_list() {
	reset("material_list");
	for (int i=0;i<data->material.num;i++) {
		int nt = 0;
		for (auto &t: data->mesh->polygon)
			if (t.material == i)
				nt ++;
		string im = render_material(data->material[i]);
		add_string("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im.c_str(), file_secure(data->material[i]->filename).c_str()));
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
	ed->error_box(_("not implemented yet"));
}

void ModelMaterialDialog::on_material_apply() {
	data->setMaterialSelection(mode_model_mesh->current_material);
}


void ModelMaterialDialog::on_default_colors() {
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
		add_string("mat_textures", format("Tex[%d]\\%s\\%s", i, id.c_str(), (file_secure(mat->texture_levels[i]->filename) + ext).c_str()));
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
			tl->image->save(nix::texture_dir + storage->dialog_file);
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
		destroy();
	}
	void on_cancel() {
		destroy();
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
	mode_model_mesh_texture->setCurrentTextureLevel(sel);
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
		mode_model_mesh_texture->setCurrentTextureLevel(n);
	}
	popup_textures->enable("texture-level-delete", n>=0);
	popup_textures->enable("texture-level-clear", n>=0);
	popup_textures->enable("texture-level-load", n>=0);
	popup_textures->enable("texture-level-save", n>=0);
	popup_textures->enable("texture-level-scale", n>=0);
	popup_textures->open_popup(this);
}

void ModelMaterialDialog::on_update(Observable *o, const string &message) {
	if (o == mode_model_mesh) {
		load_data();
	} else if (o == mode_model_mesh_texture) {
		load_data();
	} else if (o == data and message == data->MESSAGE_MATERIAL_CHANGE) {
		if (apply_queue_depth == 0)
			load_data();
	} else if (o == data and message == data->MESSAGE_TEXTURE_CHANGE) {
		if (apply_queue_depth == 0)
			load_data();
	}
}

