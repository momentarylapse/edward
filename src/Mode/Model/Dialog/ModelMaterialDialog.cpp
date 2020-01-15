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
#include "../../../lib/nix/nix.h"
#include "../Mesh/ModeModelMeshTexture.h"

string file_secure(const string &filename);
string render_material(ModelMaterial *m);

ModelMaterialDialog::ModelMaterialDialog(DataModel *_data) :
	Observer("ModelMaterialDialog")
{
	from_resource("model_material_dialog");

	data = _data;
	subscribe(data);
	subscribe(mode_model_mesh);
	subscribe(mode_model_mesh_texture);

	popup_materials = hui::CreateResourceMenu("model-material-list-popup");
	popup_textures = hui::CreateResourceMenu("model-texture-list-popup");

	set_tooltip("mat_am", _("Farbe des Objektes im Schatten (Umgebungslicht)"));
	set_tooltip("mat_di", _("Farbe des Objektes bei direktem Licht"));
	set_tooltip("mat_sp", _("Farbe von Glanzflecken"));
	set_tooltip("mat_shininess", _("H&arte des Glanzes (10=weich, 100=hart)"));
	set_tooltip("mat_em", _("Farbe des Objektes bei absoluter Dunkelheit"));

	set_tooltip("transparency_mode:color_key", _("Reines Gr&un wird transparent"));

	event("material_list", std::bind(&ModelMaterialDialog::on_material_list, this));
	event_x("material_list", "hui:select", std::bind(&ModelMaterialDialog::on_material_list_select, this));
	event_x("material_list", "hui:right-button-down", std::bind(&ModelMaterialDialog::on_material_list_right_click, this));
	event("add_new_material", std::bind(&ModelMaterialDialog::on_add_new_material, this));
	event("add_material", std::bind(&ModelMaterialDialog::on_add_material, this));
	event("delete_material", std::bind(&ModelMaterialDialog::on_delete_material, this));
	event("apply_material", std::bind(&ModelMaterialDialog::on_apply_material, this));


	event("texture-level-add", std::bind(&ModelMaterialDialog::on_texture_level_add, this));
	event("mat_textures", std::bind(&ModelMaterialDialog::on_textures, this));
	event_x("mat_textures", "hui:select", std::bind(&ModelMaterialDialog::on_textures_select, this));
	event_x("mat_textures", "hui:right-button-down", std::bind(&ModelMaterialDialog::on_textures_right_click, this));
	event("texture-level-delete", std::bind(&ModelMaterialDialog::on_texture_level_delete, this));
	event("texture-level-clear", std::bind(&ModelMaterialDialog::on_texture_level_clear, this));
	event("texture-level-load", std::bind(&ModelMaterialDialog::on_texture_level_load, this));
	event("transparency_mode:material", std::bind(&ModelMaterialDialog::on_transparency_mode, this));
	event("transparency_mode:none", std::bind(&ModelMaterialDialog::on_transparency_mode, this));
	event("transparency_mode:function", std::bind(&ModelMaterialDialog::on_transparency_mode, this));
	event("transparency_mode:color_key", std::bind(&ModelMaterialDialog::on_transparency_mode, this));
	event("transparency_mode:factor", std::bind(&ModelMaterialDialog::on_transparency_mode, this));

	event("default_colors", std::bind(&ModelMaterialDialog::on_default_colors, this));
	event("mat_am", std::bind(&ModelMaterialDialog::apply_data, this));
	event("mat_di", std::bind(&ModelMaterialDialog::apply_data, this));
	event("mat_sp", std::bind(&ModelMaterialDialog::apply_data, this));
	event("mat_em", std::bind(&ModelMaterialDialog::apply_data, this));
	event("mat_shininess", std::bind(&ModelMaterialDialog::apply_data_delayed, this));

	event("alpha_factor", std::bind(&ModelMaterialDialog::apply_data_delayed, this));
	event("alpha_source", std::bind(&ModelMaterialDialog::apply_data_delayed, this));
	event("alpha_dest", std::bind(&ModelMaterialDialog::apply_data_delayed, this));
	event("alpha_z_buffer", std::bind(&ModelMaterialDialog::apply_data, this));

	expand_all("model_material_dialog_grp_textures", true);

	load_data();
	apply_queue_depth = 0;
}

ModelMaterialDialog::~ModelMaterialDialog()
{
	unsubscribe(mode_model_mesh_texture);
	unsubscribe(mode_model_mesh);
	unsubscribe(data);

	delete popup_materials;
	delete popup_textures;
}

void ModelMaterialDialog::load_data()
{
	temp = data->material[mode_model_mesh->current_material];
	fill_material_list();

	fill_texture_list();
	check("default_colors", !temp.user_color);
	enable("mat_am", temp.user_color);
	enable("mat_di", temp.user_color);
	enable("mat_sp", temp.user_color);
	enable("mat_em", temp.user_color);
	enable("mat_shininess", temp.user_color);
	set_color("mat_am", temp.ambient);
	set_color("mat_di", temp.diffuse);
	set_color("mat_sp", temp.specular);
	set_color("mat_em", temp.emission);
	set_float("mat_shininess", temp.shininess);

	if (temp.transparency_mode == TransparencyModeColorKeySmooth)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TransparencyModeColorKeyHard)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TransparencyModeFactor)
		check("transparency_mode:factor", true);
	else if (temp.transparency_mode == TransparencyModeFunctions)
		check("transparency_mode:function", true);
	else if (temp.transparency_mode == TransparencyModeNone)
		check("transparency_mode:none", true);
	else
		check("transparency_mode:material", true);
	enable("alpha_factor", temp.transparency_mode == TransparencyModeFactor);
	enable("alpha_source", temp.transparency_mode == TransparencyModeFunctions);
	enable("alpha_dest", temp.transparency_mode == TransparencyModeFunctions);
	set_float("alpha_factor", temp.alpha_factor * 100.0f);
	check("alpha_z_buffer", temp.alpha_zbuffer);
	set_int("alpha_source", temp.alpha_source);
	set_int("alpha_dest", temp.alpha_destination);
}


void ModelMaterialDialog::apply_data()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	if (temp.user_color){
		temp.ambient = get_color("mat_am");
		temp.diffuse = get_color("mat_di");
		temp.specular = get_color("mat_sp");
		temp.emission = get_color("mat_em");
		temp.shininess = get_float("mat_shininess");
	}
	temp.alpha_zbuffer = is_checked("alpha_z_buffer");
	temp.alpha_factor = get_float("alpha_factor") * 0.01f;
	temp.alpha_source = get_int("alpha_source");
	temp.alpha_destination = get_int("alpha_dest");

	data->execute(new ActionModelEditMaterial(mode_model_mesh->current_material, temp));
}

void ModelMaterialDialog::apply_data_delayed()
{
	apply_queue_depth ++;
	hui::RunLater(0.5f, std::bind(&ModelMaterialDialog::apply_data, this));
}

void ModelMaterialDialog::fill_material_list()
{
	reset("material_list");
	for (int i=0;i<data->material.num;i++){
		int nt = 0;
		for (ModelSurface &s: data->surface)
			for (ModelPolygon &t: s.polygon)
				if (t.material == i)
					nt ++;
		string im = render_material(&data->material[i]);
		add_string("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im.c_str(), file_secure(data->material[i].material_file).c_str()));
	}
	set_int("material_list", mode_model_mesh->current_material);
	//Enable("delete_material", false);
}


// material
void ModelMaterialDialog::on_material_list()
{
	/*int s = GetInt("");
	if (s < 0)
		return;

	mode_model_mesh->CurrentMaterial = s;
	mode_model_mesh_texture->CurrentTextureLevel = 0;
	mode_model->ExecuteMaterialDialog(0);*/
}

void ModelMaterialDialog::on_material_list_select()
{
	mode_model_mesh->setCurrentMaterial(get_int(""));
}

void ModelMaterialDialog::on_add_new_material()
{
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialDialog::on_add_material()
{
	if (ed->file_dialog(FD_MATERIAL, false, true))
		data->execute(new ActionModelAddMaterial(ed->dialog_file_no_ending));
}

void ModelMaterialDialog::on_delete_material()
{
	hui::ErrorBox(win, "", "noch nicht implementiert");
}

void ModelMaterialDialog::on_apply_material()
{
	data->setMaterialSelection(mode_model_mesh->current_material);
}


void ModelMaterialDialog::on_default_colors()
{
	temp.user_color = !is_checked("");
	if (!temp.user_color)
		temp.checkColors();
	apply_data();
}

void ModelMaterialDialog::fill_texture_list()
{
	reset("mat_textures");
	for (int i=0;i<temp.texture_files.num;i++){
		nix::Texture *tex = nix::LoadTexture(temp.texture_files[i]);
		string img = ed->get_tex_image(tex);
		add_string("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.texture_files[i]).c_str()));
	}
	set_int("mat_textures", mode_model_mesh_texture->current_texture_level);
}


void ModelMaterialDialog::on_texture_level_add()
{
	if (temp.texture_files.num >= MATERIAL_MAX_TEXTURES){
		ed->error_box(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.texture_files.add("");
	temp.checkTextures();
	apply_data();
}

void ModelMaterialDialog::on_textures()
{
	on_textures_select();
	on_texture_level_load();
}

void ModelMaterialDialog::on_texture_level_load()
{
	int sel = get_int("mat_textures");
	if (sel >= 0)
		if (ed->file_dialog(FD_TEXTURE, false, true)){
			temp.texture_files[sel] = ed->dialog_file;
			temp.checkTextures();
			apply_data();
		}
}

void ModelMaterialDialog::on_textures_select() {
	int sel = get_int("");
	mode_model_mesh_texture->setCurrentTextureLevel(sel);
}

void ModelMaterialDialog::on_texture_level_delete() {
	int sel = get_int("mat_textures");
	if (sel >= 0){
		if (temp.texture_files.num <= 1){
			ed->error_box(_("Mindestens eine Textur-Ebene muss vorhanden sein!"));
			return;
		}
		temp.texture_files.erase(sel);
		apply_data();
		fill_texture_list();
	}
}

void ModelMaterialDialog::on_texture_level_clear()
{
	int sel = get_int("mat_textures");
	if (sel >= 0){
		temp.texture_files[sel] = "";
		apply_data();
		fill_texture_list();
	}
}

void ModelMaterialDialog::on_transparency_mode()
{
	if (is_checked("transparency_mode:function"))
		temp.transparency_mode = TransparencyModeFunctions;
	else if (is_checked("transparency_mode:color_key"))
		temp.transparency_mode = TransparencyModeColorKeyHard;
	else if (is_checked("transparency_mode:factor"))
		temp.transparency_mode = TransparencyModeFactor;
	else if (is_checked("transparency_mode:none"))
		temp.transparency_mode = TransparencyModeNone;
	else
		temp.transparency_mode = TransparencyModeDefault;
	enable("alpha_factor", temp.transparency_mode == TransparencyModeFactor);
	enable("alpha_source", temp.transparency_mode == TransparencyModeFunctions);
	enable("alpha_dest", temp.transparency_mode == TransparencyModeFunctions);
	apply_data();
}

void ModelMaterialDialog::on_material_list_right_click() {
	int n = hui::GetEvent()->row;
	if (n >= 0) {
		mode_model_mesh->setCurrentMaterial(n);
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
	popup_textures->open_popup(this);
}

void ModelMaterialDialog::on_update(Observable *o, const string &message)
{
	load_data();
}

