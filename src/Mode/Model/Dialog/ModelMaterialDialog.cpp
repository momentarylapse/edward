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

string file_secure(const string &filename);
string render_material(ModelMaterial *m);

ModelMaterialDialog::ModelMaterialDialog(HuiWindow *_parent, DataModel *_data) :
	EmbeddedDialog(_parent, "model_material_dialog", "root-table", 1, 0, "noexpandx")
{
	data = _data;
	temp = data->Material[mode_model_mesh->CurrentMaterial];
	Subscribe(data);
	Subscribe(mode_model_mesh);
	Subscribe(mode_model_mesh_texture);


	win->SetTooltip("mat_textures", _("&Ubereinanderliegende Textur-Schichten (multitexturing)\n- Doppelklick um eine Texturdatei zu w&ahlen"));
	win->SetTooltip("mat_add_texture_level", _("f&ugt eine weitere Textur-Schicht hinzu (multitexturing)"));
	win->SetTooltip("mat_delete_texture_level", _("l&oscht die ausgew&ahlte Textur-Schicht"));
	win->SetTooltip("mat_empty_texture_level", _("l&oscht die Textur der ausgew&ahlten Textur-Schicht"));

	win->SetTooltip("mat_am", _("Farbe des Objektes im Schatten (Umgebungslicht)"));
	win->SetTooltip("mat_di", _("Farbe des Objektes bei direktem Licht"));
	win->SetTooltip("mat_sp", _("Farbe von Glanzflecken"));
	win->SetTooltip("mat_shininess", _("H&arte des Glanzes (10=weich, 100=hart)"));
	win->SetTooltip("mat_em", _("Farbe des Objektes bei absoluter Dunkelheit"));

	win->SetTooltip("transparency_mode:color_key", _("Reines Gr&un wird transparent"));

	win->EventM("material_list", this, &ModelMaterialDialog::OnMaterialList);
	win->EventMX("material_list", "hui:select", this, &ModelMaterialDialog::OnMaterialListSelect);
	win->EventM("add_new_material", this, &ModelMaterialDialog::OnAddNewMaterial);
	win->EventM("add_material", this, &ModelMaterialDialog::OnAddMaterial);
	win->EventM("delete_material", this, &ModelMaterialDialog::OnDeleteMaterial);
	win->EventM("apply_material", this, &ModelMaterialDialog::OnApplyMaterial);


	win->EventM("mat_add_texture_level", this, &ModelMaterialDialog::OnAddTextureLevel);
	win->EventM("mat_textures", this, &ModelMaterialDialog::OnTextures);
	win->EventMX("mat_textures", "hui:select", this, &ModelMaterialDialog::OnTexturesSelect);
	win->EventM("mat_delete_texture_level", this, &ModelMaterialDialog::OnDeleteTextureLevel);
	win->EventM("mat_empty_texture_level", this, &ModelMaterialDialog::OnEmptyTextureLevel);
	win->EventM("transparency_mode:material", this, &ModelMaterialDialog::OnTransparencyMode);
	win->EventM("transparency_mode:none", this, &ModelMaterialDialog::OnTransparencyMode);
	win->EventM("transparency_mode:function", this, &ModelMaterialDialog::OnTransparencyMode);
	win->EventM("transparency_mode:color_key", this, &ModelMaterialDialog::OnTransparencyMode);
	win->EventM("transparency_mode:factor", this, &ModelMaterialDialog::OnTransparencyMode);

	win->EventM("default_colors", this, &ModelMaterialDialog::OnDefaultColors);
	win->EventM("mat_am", this, &ModelMaterialDialog::ApplyData);
	win->EventM("mat_di", this, &ModelMaterialDialog::ApplyData);
	win->EventM("mat_sp", this, &ModelMaterialDialog::ApplyData);
	win->EventM("mat_em", this, &ModelMaterialDialog::ApplyData);
	win->EventM("mat_shininess", this, &ModelMaterialDialog::ApplyDataDelayed);

	win->EventM("alpha_factor", this, &ModelMaterialDialog::ApplyDataDelayed);
	win->EventM("alpha_source", this, &ModelMaterialDialog::ApplyDataDelayed);
	win->EventM("alpha_dest", this, &ModelMaterialDialog::ApplyDataDelayed);
	win->EventM("alpha_z_buffer", this, &ModelMaterialDialog::ApplyData);

	win->ExpandAll("model_material_dialog_grp_textures", true);

	LoadData();
	apply_queue_depth = 0;
}

ModelMaterialDialog::~ModelMaterialDialog()
{
	Unsubscribe(mode_model_mesh_texture);
	Unsubscribe(mode_model_mesh);
	Unsubscribe(data);
}

void ModelMaterialDialog::LoadData()
{
	temp = data->Material[mode_model_mesh->CurrentMaterial];
	FillMaterialList();

	FillTextureList();
	Check("default_colors", !temp.UserColor);
	Enable("mat_am", temp.UserColor);
	Enable("mat_di", temp.UserColor);
	Enable("mat_sp", temp.UserColor);
	Enable("mat_em", temp.UserColor);
	Enable("mat_shininess", temp.UserColor);
	win->SetColor("mat_am", temp.Ambient);
	win->SetColor("mat_di", temp.Diffuse);
	win->SetColor("mat_sp", temp.Specular);
	win->SetColor("mat_em", temp.Emission);
	SetFloat("mat_shininess", temp.Shininess);

	if (temp.TransparencyMode == TransparencyModeColorKeySmooth)
		Check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeColorKeyHard)
		Check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeFactor)
		Check("transparency_mode:factor", true);
	else if (temp.TransparencyMode == TransparencyModeFunctions)
		Check("transparency_mode:function", true);
	else if (temp.TransparencyMode == TransparencyModeNone)
		Check("transparency_mode:none", true);
	else
		Check("transparency_mode:material", true);
	Enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	Enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	Enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	SetFloat("alpha_factor", temp.AlphaFactor * 100.0f);
	Check("alpha_z_buffer", temp.AlphaZBuffer);
	SetInt("alpha_source", temp.AlphaSource);
	SetInt("alpha_dest", temp.AlphaDestination);
}


void ModelMaterialDialog::ApplyData()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	if (temp.UserColor){
		temp.Ambient = win->GetColor("mat_am");
		temp.Diffuse = win->GetColor("mat_di");
		temp.Specular = win->GetColor("mat_sp");
		temp.Emission = win->GetColor("mat_em");
		temp.Shininess = GetFloat("mat_shininess");
	}
	temp.AlphaZBuffer = IsChecked("alpha_z_buffer");
	temp.AlphaFactor = GetFloat("alpha_factor") * 0.01f;
	temp.AlphaSource = GetInt("alpha_source");
	temp.AlphaDestination = GetInt("alpha_dest");

	data->Execute(new ActionModelEditMaterial(mode_model_mesh->CurrentMaterial, temp));
}

void ModelMaterialDialog::ApplyDataDelayed()
{
	apply_queue_depth ++;
	HuiRunLaterM(0.5f, this, &ModelMaterialDialog::ApplyData);
}

void ModelMaterialDialog::FillMaterialList()
{
	Reset("material_list");
	for (int i=0;i<data->Material.num;i++){
		int nt = 0;
		foreach(ModelSurface &s, data->Surface)
			foreach(ModelPolygon &t, s.Polygon)
				if (t.Material == i)
					nt ++;
		string im = render_material(&data->Material[i]);
		AddString("material_list", format("Mat[%d]\\%d\\%s\\%s", i, nt, im.c_str(), file_secure(data->Material[i].MaterialFile).c_str()));
	}
	SetInt("material_list", mode_model_mesh->CurrentMaterial);
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
	mode_model_mesh->SetCurrentMaterial(GetInt(""));
}

void ModelMaterialDialog::OnAddNewMaterial()
{
	data->Execute(new ActionModelAddMaterial(""));
}

void ModelMaterialDialog::OnAddMaterial()
{
	if (ed->FileDialog(FDMaterial, false, true))
		data->Execute(new ActionModelAddMaterial(ed->DialogFileNoEnding));
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
	temp.UserColor = !IsChecked("");
	if (!temp.UserColor)
		temp.CheckColors();
	ApplyData();
}

void ModelMaterialDialog::FillTextureList()
{
	Reset("mat_textures");
	for (int i=0;i<temp.NumTextures;i++){
		NixTexture *tex = NixLoadTexture(temp.TextureFile[i]);
		string img = ed->get_tex_image(tex);
		AddString("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	SetInt("mat_textures", mode_model_mesh_texture->CurrentTextureLevel);
}


void ModelMaterialDialog::OnAddTextureLevel()
{
	if (temp.NumTextures >= MATERIAL_MAX_TEXTURES){
		ed->ErrorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.TextureFile[temp.NumTextures ++] = "";
	temp.CheckTextures();
	ApplyData();
}

void ModelMaterialDialog::OnTextures()
{
	int sel = GetInt("");
	if ((sel >= 0) && (sel <temp.NumTextures))
		if (ed->FileDialog(FDTexture, false, true)){
			temp.TextureFile[sel] = ed->DialogFile;
			temp.CheckTextures();
			ApplyData();
		}
}

void ModelMaterialDialog::OnTexturesSelect()
{
	int sel = GetInt("");
	mode_model_mesh_texture->SetCurrentTextureLevel(sel);
}

void ModelMaterialDialog::OnDeleteTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		if (temp.NumTextures <= 1){
			ed->ErrorBox(_("Mindestens eine Textur-Ebene muss vorhanden sein!"));
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
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		temp.TextureFile[sel] = "";
		ApplyData();
		FillTextureList();
	}
}

void ModelMaterialDialog::OnTransparencyMode()
{
	if (IsChecked("transparency_mode:function"))
		temp.TransparencyMode = TransparencyModeFunctions;
	else if (IsChecked("transparency_mode:color_key"))
		temp.TransparencyMode = TransparencyModeColorKeyHard;
	else if (IsChecked("transparency_mode:factor"))
		temp.TransparencyMode = TransparencyModeFactor;
	else if (IsChecked("transparency_mode:none"))
		temp.TransparencyMode = TransparencyModeNone;
	else
		temp.TransparencyMode = TransparencyModeDefault;
	Enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	Enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	Enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	ApplyData();
}

void ModelMaterialDialog::OnUpdate(Observable *o)
{
	LoadData();
}

