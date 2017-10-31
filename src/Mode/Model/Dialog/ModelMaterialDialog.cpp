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

ModelMaterialDialog::ModelMaterialDialog(hui::Window *_parent, DataModel *_data) :
	//EmbeddedDialog(_parent, "model_material_dialog", "root-table", 1, 0, "noexpandx"),
	Observer("ModelMaterialDialog")
{
	fromResource("model_material_dialog");

	data = _data;
	subscribe(data);
	subscribe(mode_model_mesh);
	subscribe(mode_model_mesh_texture);


	setTooltip("mat_textures", _("&Ubereinanderliegende Textur-Schichten (multitexturing)\n- Doppelklick um eine Texturdatei zu w&ahlen"));
	setTooltip("mat_add_texture_level", _("f&ugt eine weitere Textur-Schicht hinzu (multitexturing)"));
	setTooltip("mat_delete_texture_level", _("l&oscht die ausgew&ahlte Textur-Schicht"));
	setTooltip("mat_empty_texture_level", _("l&oscht die Textur der ausgew&ahlten Textur-Schicht"));

	setTooltip("mat_am", _("Farbe des Objektes im Schatten (Umgebungslicht)"));
	setTooltip("mat_di", _("Farbe des Objektes bei direktem Licht"));
	setTooltip("mat_sp", _("Farbe von Glanzflecken"));
	setTooltip("mat_shininess", _("H&arte des Glanzes (10=weich, 100=hart)"));
	setTooltip("mat_em", _("Farbe des Objektes bei absoluter Dunkelheit"));

	setTooltip("transparency_mode:color_key", _("Reines Gr&un wird transparent"));

	event("material_list", std::bind(&ModelMaterialDialog::onMaterialList, this));
	eventX("material_list", "hui:select", std::bind(&ModelMaterialDialog::onMaterialListSelect, this));
	event("add_new_material", std::bind(&ModelMaterialDialog::onAddNewMaterial, this));
	event("add_material", std::bind(&ModelMaterialDialog::onAddMaterial, this));
	event("delete_material", std::bind(&ModelMaterialDialog::onDeleteMaterial, this));
	event("apply_material", std::bind(&ModelMaterialDialog::onApplyMaterial, this));


	event("mat_add_texture_level", std::bind(&ModelMaterialDialog::onAddTextureLevel, this));
	event("mat_textures", std::bind(&ModelMaterialDialog::onTextures, this));
	eventX("mat_textures", "hui:select", std::bind(&ModelMaterialDialog::onTexturesSelect, this));
	event("mat_delete_texture_level", std::bind(&ModelMaterialDialog::onDeleteTextureLevel, this));
	event("mat_empty_texture_level", std::bind(&ModelMaterialDialog::onEmptyTextureLevel, this));
	event("transparency_mode:material", std::bind(&ModelMaterialDialog::onTransparencyMode, this));
	event("transparency_mode:none", std::bind(&ModelMaterialDialog::onTransparencyMode, this));
	event("transparency_mode:function", std::bind(&ModelMaterialDialog::onTransparencyMode, this));
	event("transparency_mode:color_key", std::bind(&ModelMaterialDialog::onTransparencyMode, this));
	event("transparency_mode:factor", std::bind(&ModelMaterialDialog::onTransparencyMode, this));

	event("default_colors", std::bind(&ModelMaterialDialog::onDefaultColors, this));
	event("mat_am", std::bind(&ModelMaterialDialog::applyData, this));
	event("mat_di", std::bind(&ModelMaterialDialog::applyData, this));
	event("mat_sp", std::bind(&ModelMaterialDialog::applyData, this));
	event("mat_em", std::bind(&ModelMaterialDialog::applyData, this));
	event("mat_shininess", std::bind(&ModelMaterialDialog::applyDataDelayed, this));

	event("alpha_factor", std::bind(&ModelMaterialDialog::applyDataDelayed, this));
	event("alpha_source", std::bind(&ModelMaterialDialog::applyDataDelayed, this));
	event("alpha_dest", std::bind(&ModelMaterialDialog::applyDataDelayed, this));
	event("alpha_z_buffer", std::bind(&ModelMaterialDialog::applyData, this));

	expandAll("model_material_dialog_grp_textures", true);

	loadData();
	apply_queue_depth = 0;
}

ModelMaterialDialog::~ModelMaterialDialog()
{
	unsubscribe(mode_model_mesh_texture);
	unsubscribe(mode_model_mesh);
	unsubscribe(data);
}

void ModelMaterialDialog::loadData()
{
	temp = data->material[mode_model_mesh->current_material];
	fillMaterialList();

	fillTextureList();
	check("default_colors", !temp.user_color);
	enable("mat_am", temp.user_color);
	enable("mat_di", temp.user_color);
	enable("mat_sp", temp.user_color);
	enable("mat_em", temp.user_color);
	enable("mat_shininess", temp.user_color);
	setColor("mat_am", temp.ambient);
	setColor("mat_di", temp.diffuse);
	setColor("mat_sp", temp.specular);
	setColor("mat_em", temp.emission);
	setFloat("mat_shininess", temp.shininess);

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
	setFloat("alpha_factor", temp.alpha_factor * 100.0f);
	check("alpha_z_buffer", temp.alpha_zbuffer);
	setInt("alpha_source", temp.alpha_source);
	setInt("alpha_dest", temp.alpha_destination);
}


void ModelMaterialDialog::applyData()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	if (temp.user_color){
		temp.ambient = getColor("mat_am");
		temp.diffuse = getColor("mat_di");
		temp.specular = getColor("mat_sp");
		temp.emission = getColor("mat_em");
		temp.shininess = getFloat("mat_shininess");
	}
	temp.alpha_zbuffer = isChecked("alpha_z_buffer");
	temp.alpha_factor = getFloat("alpha_factor") * 0.01f;
	temp.alpha_source = getInt("alpha_source");
	temp.alpha_destination = getInt("alpha_dest");

	data->execute(new ActionModelEditMaterial(mode_model_mesh->current_material, temp));
}

void ModelMaterialDialog::applyDataDelayed()
{
	apply_queue_depth ++;
	hui::RunLater(0.5f, std::bind(&ModelMaterialDialog::applyData, this));
}

void ModelMaterialDialog::fillMaterialList()
{
	reset("material_list");
	for (int i=0;i<data->material.num;i++){
		int nt = 0;
		for (ModelSurface &s: data->surface)
			for (ModelPolygon &t: s.polygon)
				if (t.material == i)
					nt ++;
		string im = render_material(&data->material[i]);
		addString("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im.c_str(), file_secure(data->material[i].material_file).c_str()));
	}
	setInt("material_list", mode_model_mesh->current_material);
	//Enable("delete_material", false);
}


// material
void ModelMaterialDialog::onMaterialList()
{
	/*int s = GetInt("");
	if (s < 0)
		return;

	mode_model_mesh->CurrentMaterial = s;
	mode_model_mesh_texture->CurrentTextureLevel = 0;
	mode_model->ExecuteMaterialDialog(0);*/
}

void ModelMaterialDialog::onMaterialListSelect()
{
	mode_model_mesh->setCurrentMaterial(getInt(""));
}

void ModelMaterialDialog::onAddNewMaterial()
{
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialDialog::onAddMaterial()
{
	if (ed->fileDialog(FD_MATERIAL, false, true))
		data->execute(new ActionModelAddMaterial(ed->dialog_file_no_ending));
}

void ModelMaterialDialog::onDeleteMaterial()
{
	hui::ErrorBox(win, "", "noch nicht implementiert");
}

void ModelMaterialDialog::onApplyMaterial()
{
	data->setMaterialSelection(mode_model_mesh->current_material);
}


void ModelMaterialDialog::onDefaultColors()
{
	temp.user_color = !isChecked("");
	if (!temp.user_color)
		temp.checkColors();
	applyData();
}

void ModelMaterialDialog::fillTextureList()
{
	reset("mat_textures");
	for (int i=0;i<temp.texture_files.num;i++){
		nix::Texture *tex = nix::LoadTexture(temp.texture_files[i]);
		string img = ed->get_tex_image(tex);
		addString("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.texture_files[i]).c_str()));
	}
	setInt("mat_textures", mode_model_mesh_texture->current_texture_level);
}


void ModelMaterialDialog::onAddTextureLevel()
{
	if (temp.texture_files.num >= MATERIAL_MAX_TEXTURES){
		ed->errorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.texture_files.add("");
	temp.checkTextures();
	applyData();
}

void ModelMaterialDialog::onTextures()
{
	int sel = getInt("");
	if ((sel >= 0) && (sel <temp.texture_files.num))
		if (ed->fileDialog(FD_TEXTURE, false, true)){
			temp.texture_files[sel] = ed->dialog_file;
			temp.checkTextures();
			applyData();
		}
}

void ModelMaterialDialog::onTexturesSelect()
{
	int sel = getInt("");
	mode_model_mesh_texture->setCurrentTextureLevel(sel);
}

void ModelMaterialDialog::onDeleteTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		if (temp.texture_files.num <= 1){
			ed->errorBox(_("Mindestens eine Textur-Ebene muss vorhanden sein!"));
			return;
		}
		temp.texture_files.erase(sel);
		applyData();
		fillTextureList();
	}
}

void ModelMaterialDialog::onEmptyTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		temp.texture_files[sel] = "";
		applyData();
		fillTextureList();
	}
}

void ModelMaterialDialog::onTransparencyMode()
{
	if (isChecked("transparency_mode:function"))
		temp.transparency_mode = TransparencyModeFunctions;
	else if (isChecked("transparency_mode:color_key"))
		temp.transparency_mode = TransparencyModeColorKeyHard;
	else if (isChecked("transparency_mode:factor"))
		temp.transparency_mode = TransparencyModeFactor;
	else if (isChecked("transparency_mode:none"))
		temp.transparency_mode = TransparencyModeNone;
	else
		temp.transparency_mode = TransparencyModeDefault;
	enable("alpha_factor", temp.transparency_mode == TransparencyModeFactor);
	enable("alpha_source", temp.transparency_mode == TransparencyModeFunctions);
	enable("alpha_dest", temp.transparency_mode == TransparencyModeFunctions);
	applyData();
}

void ModelMaterialDialog::onUpdate(Observable *o, const string &message)
{
	loadData();
}

