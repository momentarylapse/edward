/*
 * ModelMaterialDialog.cpp
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#include "ModelMaterialDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../Mesh/ModeModelMesh.h"
#include "../Mesh/ModeModelMeshTexture.h"
#include "../../../Action/Model/Data/ActionModelAddMaterial.h"
#include "../../../Action/Model/Data/ActionModelEditMaterial.h"
#include "../../../Action/Model/Data/ActionModelEditData.h"
#include "../../../Edward.h"
#include "../../../lib/nix/nix.h"

string file_secure(const string &filename);
string render_material(ModelMaterial *m);

ModelMaterialDialog::ModelMaterialDialog(HuiWindow *_parent, DataModel *_data) :
	EmbeddedDialog(_parent, "model_material_dialog", "root-table", 1, 0, "noexpandx"),
	Observer("ModelMaterialDialog")
{
	data = _data;
	temp = data->Material[mode_model_mesh->CurrentMaterial];
	subscribe(data);
	subscribe(mode_model_mesh);
	subscribe(mode_model_mesh_texture);


	win->setTooltip("mat_textures", _("&Ubereinanderliegende Textur-Schichten (multitexturing)\n- Doppelklick um eine Texturdatei zu w&ahlen"));
	win->setTooltip("mat_add_texture_level", _("f&ugt eine weitere Textur-Schicht hinzu (multitexturing)"));
	win->setTooltip("mat_delete_texture_level", _("l&oscht die ausgew&ahlte Textur-Schicht"));
	win->setTooltip("mat_empty_texture_level", _("l&oscht die Textur der ausgew&ahlten Textur-Schicht"));

	win->setTooltip("mat_am", _("Farbe des Objektes im Schatten (Umgebungslicht)"));
	win->setTooltip("mat_di", _("Farbe des Objektes bei direktem Licht"));
	win->setTooltip("mat_sp", _("Farbe von Glanzflecken"));
	win->setTooltip("mat_shininess", _("H&arte des Glanzes (10=weich, 100=hart)"));
	win->setTooltip("mat_em", _("Farbe des Objektes bei absoluter Dunkelheit"));

	win->setTooltip("transparency_mode:color_key", _("Reines Gr&un wird transparent"));

	win->event("material_list", this, &ModelMaterialDialog::OnMaterialList);
	win->eventX("material_list", "hui:select", this, &ModelMaterialDialog::OnMaterialListSelect);
	win->event("add_new_material", this, &ModelMaterialDialog::OnAddNewMaterial);
	win->event("add_material", this, &ModelMaterialDialog::OnAddMaterial);
	win->event("delete_material", this, &ModelMaterialDialog::OnDeleteMaterial);
	win->event("apply_material", this, &ModelMaterialDialog::OnApplyMaterial);


	win->event("mat_add_texture_level", this, &ModelMaterialDialog::OnAddTextureLevel);
	win->event("mat_textures", this, &ModelMaterialDialog::OnTextures);
	win->eventX("mat_textures", "hui:select", this, &ModelMaterialDialog::OnTexturesSelect);
	win->event("mat_delete_texture_level", this, &ModelMaterialDialog::OnDeleteTextureLevel);
	win->event("mat_empty_texture_level", this, &ModelMaterialDialog::OnEmptyTextureLevel);
	win->event("transparency_mode:material", this, &ModelMaterialDialog::OnTransparencyMode);
	win->event("transparency_mode:none", this, &ModelMaterialDialog::OnTransparencyMode);
	win->event("transparency_mode:function", this, &ModelMaterialDialog::OnTransparencyMode);
	win->event("transparency_mode:color_key", this, &ModelMaterialDialog::OnTransparencyMode);
	win->event("transparency_mode:factor", this, &ModelMaterialDialog::OnTransparencyMode);

	win->event("default_colors", this, &ModelMaterialDialog::OnDefaultColors);
	win->event("mat_am", this, &ModelMaterialDialog::ApplyData);
	win->event("mat_di", this, &ModelMaterialDialog::ApplyData);
	win->event("mat_sp", this, &ModelMaterialDialog::ApplyData);
	win->event("mat_em", this, &ModelMaterialDialog::ApplyData);
	win->event("mat_shininess", this, &ModelMaterialDialog::ApplyDataDelayed);

	win->event("alpha_factor", this, &ModelMaterialDialog::ApplyDataDelayed);
	win->event("alpha_source", this, &ModelMaterialDialog::ApplyDataDelayed);
	win->event("alpha_dest", this, &ModelMaterialDialog::ApplyDataDelayed);
	win->event("alpha_z_buffer", this, &ModelMaterialDialog::ApplyData);

	win->expandAll("model_material_dialog_grp_textures", true);

	LoadData();
	apply_queue_depth = 0;
}

ModelMaterialDialog::~ModelMaterialDialog()
{
	unsubscribe(mode_model_mesh_texture);
	unsubscribe(mode_model_mesh);
	unsubscribe(data);
}

void ModelMaterialDialog::LoadData()
{
	temp = data->Material[mode_model_mesh->CurrentMaterial];
	FillMaterialList();

	FillTextureList();
	check("default_colors", !temp.UserColor);
	enable("mat_am", temp.UserColor);
	enable("mat_di", temp.UserColor);
	enable("mat_sp", temp.UserColor);
	enable("mat_em", temp.UserColor);
	enable("mat_shininess", temp.UserColor);
	win->setColor("mat_am", temp.Ambient);
	win->setColor("mat_di", temp.Diffuse);
	win->setColor("mat_sp", temp.Specular);
	win->setColor("mat_em", temp.Emission);
	setFloat("mat_shininess", temp.Shininess);

	if (temp.TransparencyMode == TransparencyModeColorKeySmooth)
		check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeColorKeyHard)
		check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeFactor)
		check("transparency_mode:factor", true);
	else if (temp.TransparencyMode == TransparencyModeFunctions)
		check("transparency_mode:function", true);
	else if (temp.TransparencyMode == TransparencyModeNone)
		check("transparency_mode:none", true);
	else
		check("transparency_mode:material", true);
	enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	setFloat("alpha_factor", temp.AlphaFactor * 100.0f);
	check("alpha_z_buffer", temp.AlphaZBuffer);
	setInt("alpha_source", temp.AlphaSource);
	setInt("alpha_dest", temp.AlphaDestination);
}


void ModelMaterialDialog::ApplyData()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	if (temp.UserColor){
		temp.Ambient = win->getColor("mat_am");
		temp.Diffuse = win->getColor("mat_di");
		temp.Specular = win->getColor("mat_sp");
		temp.Emission = win->getColor("mat_em");
		temp.Shininess = getFloat("mat_shininess");
	}
	temp.AlphaZBuffer = isChecked("alpha_z_buffer");
	temp.AlphaFactor = getFloat("alpha_factor") * 0.01f;
	temp.AlphaSource = getInt("alpha_source");
	temp.AlphaDestination = getInt("alpha_dest");

	data->execute(new ActionModelEditMaterial(mode_model_mesh->CurrentMaterial, temp));
}

void ModelMaterialDialog::ApplyDataDelayed()
{
	apply_queue_depth ++;
	HuiRunLaterM(0.5f, this, &ModelMaterialDialog::ApplyData);
}

void ModelMaterialDialog::FillMaterialList()
{
	reset("material_list");
	for (int i=0;i<data->Material.num;i++){
		int nt = 0;
		foreach(ModelSurface &s, data->Surface)
			foreach(ModelPolygon &t, s.Polygon)
				if (t.Material == i)
					nt ++;
		string im = render_material(&data->Material[i]);
		addString("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im.c_str(), file_secure(data->Material[i].MaterialFile).c_str()));
	}
	setInt("material_list", mode_model_mesh->CurrentMaterial);
	//Enable("delete_material", false);
}


// material
void ModelMaterialDialog::OnMaterialList()
{
	/*int s = GetInt("");
	if (s < 0)
		return;

	mode_model_mesh->CurrentMaterial = s;
	mode_model_mesh_texture->CurrentTextureLevel = 0;
	mode_model->ExecuteMaterialDialog(0);*/
}

void ModelMaterialDialog::OnMaterialListSelect()
{
	mode_model_mesh->SetCurrentMaterial(getInt(""));
}

void ModelMaterialDialog::OnAddNewMaterial()
{
	data->execute(new ActionModelAddMaterial(""));
}

void ModelMaterialDialog::OnAddMaterial()
{
	if (ed->fileDialog(FDMaterial, false, true))
		data->execute(new ActionModelAddMaterial(ed->DialogFileNoEnding));
}

void ModelMaterialDialog::OnDeleteMaterial()
{
	HuiErrorBox(win, "", "noch nicht implementiert");
}

void ModelMaterialDialog::OnApplyMaterial()
{
	data->SetMaterialSelection(mode_model_mesh->CurrentMaterial);
}


void ModelMaterialDialog::OnDefaultColors()
{
	temp.UserColor = !isChecked("");
	if (!temp.UserColor)
		temp.CheckColors();
	ApplyData();
}

void ModelMaterialDialog::FillTextureList()
{
	reset("mat_textures");
	for (int i=0;i<temp.NumTextures;i++){
		NixTexture *tex = NixLoadTexture(temp.TextureFile[i]);
		string img = ed->get_tex_image(tex);
		addString("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	setInt("mat_textures", mode_model_mesh_texture->CurrentTextureLevel);
}


void ModelMaterialDialog::OnAddTextureLevel()
{
	if (temp.NumTextures >= MATERIAL_MAX_TEXTURES){
		ed->errorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.TextureFile[temp.NumTextures ++] = "";
	temp.CheckTextures();
	ApplyData();
}

void ModelMaterialDialog::OnTextures()
{
	int sel = getInt("");
	if ((sel >= 0) && (sel <temp.NumTextures))
		if (ed->fileDialog(FDTexture, false, true)){
			temp.TextureFile[sel] = ed->DialogFile;
			temp.CheckTextures();
			ApplyData();
		}
}

void ModelMaterialDialog::OnTexturesSelect()
{
	int sel = getInt("");
	mode_model_mesh_texture->SetCurrentTextureLevel(sel);
}

void ModelMaterialDialog::OnDeleteTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		if (temp.NumTextures <= 1){
			ed->errorBox(_("Mindestens eine Textur-Ebene muss vorhanden sein!"));
			return;
		}
		for (int i=sel;i<temp.NumTextures-1;i++)
			temp.TextureFile[i] = temp.TextureFile[i + 1];
		temp.NumTextures --;
		ApplyData();
		FillTextureList();
	}
}

void ModelMaterialDialog::OnEmptyTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		temp.TextureFile[sel] = "";
		ApplyData();
		FillTextureList();
	}
}

void ModelMaterialDialog::OnTransparencyMode()
{
	if (isChecked("transparency_mode:function"))
		temp.TransparencyMode = TransparencyModeFunctions;
	else if (isChecked("transparency_mode:color_key"))
		temp.TransparencyMode = TransparencyModeColorKeyHard;
	else if (isChecked("transparency_mode:factor"))
		temp.TransparencyMode = TransparencyModeFactor;
	else if (isChecked("transparency_mode:none"))
		temp.TransparencyMode = TransparencyModeNone;
	else
		temp.TransparencyMode = TransparencyModeDefault;
	enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	ApplyData();
}

void ModelMaterialDialog::onUpdate(Observable *o, const string &message)
{
	LoadData();
}

