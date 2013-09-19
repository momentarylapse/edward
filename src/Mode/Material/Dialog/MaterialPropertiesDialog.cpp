/*
 * MaterialPropertiesDialog.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "MaterialPropertiesDialog.h"
#include "../../../Edward.h"
#include "../ModeMaterial.h"
#include "../../../Action/Material/ActionMaterialEditAppearance.h"
#include "../../../Action/Material/ActionMaterialEditPhysics.h"


string file_secure(const string &filename); // -> ModelPropertiesDialog

extern string NixShaderError; // -> nix

MaterialPropertiesDialog::MaterialPropertiesDialog(HuiWindow *_parent, DataMaterial *_data):
	EmbeddedDialog(_parent, "material_dialog", "root-table", 1, 0, "noexpandx")
{
	data = _data;

	// dialog
	win->EventM("mat_add_texture_level", this, &MaterialPropertiesDialog::OnAddTextureLevel);
	win->EventM("mat_textures", this, &MaterialPropertiesDialog::OnTextures);
	win->EventMX("mat_textures", "hui:select", this, &MaterialPropertiesDialog::OnTexturesSelect);
	win->EventM("mat_delete_texture_level", this, &MaterialPropertiesDialog::OnDeleteTextureLevel);
	win->EventM("mat_empty_texture_level", this, &MaterialPropertiesDialog::OnEmptyTextureLevel);
	win->EventM("transparency_mode:none", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->EventM("transparency_mode:function", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->EventM("transparency_mode:color_key", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->EventM("transparency_mode:factor", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->EventM("reflection_mode:none", this, &MaterialPropertiesDialog::OnReflectionMode);
	win->EventM("reflection_mode:cube_static", this, &MaterialPropertiesDialog::OnReflectionMode);
	win->EventM("reflection_mode:cube_dynamic", this, &MaterialPropertiesDialog::OnReflectionMode);
	win->EventM("reflection_textures", this, &MaterialPropertiesDialog::OnReflectionTextures);
	win->EventM("find_shader", this, &MaterialPropertiesDialog::OnFindShader);


	win->EventM("mat_am", this, &MaterialPropertiesDialog::ApplyData);
	win->EventM("mat_di", this, &MaterialPropertiesDialog::ApplyData);
	win->EventM("mat_sp", this, &MaterialPropertiesDialog::ApplyData);
	win->EventM("mat_em", this, &MaterialPropertiesDialog::ApplyData);
	win->EventM("mat_shininess", this, &MaterialPropertiesDialog::ApplyDataDelayed);

	win->EventM("alpha_factor", this, &MaterialPropertiesDialog::ApplyDataDelayed);
	win->EventM("alpha_source", this, &MaterialPropertiesDialog::ApplyDataDelayed);
	win->EventM("alpha_dest", this, &MaterialPropertiesDialog::ApplyDataDelayed);
	win->EventM("alpha_z_buffer", this, &MaterialPropertiesDialog::ApplyData);

	win->EventM("rcjump", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);
	win->EventM("rcstatic", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);
	win->EventM("rcsliding", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);
	win->EventM("rcroll", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);

	win->Expand("material_dialog_grp_color", 0, true);

	temp = data->Appearance;
	temp_phys = data->Physics;
	apply_queue_depth = 0;
	apply_phys_queue_depth = 0;
	LoadData();
	Subscribe(data);
}

void MaterialPropertiesDialog::LoadData()
{
	FillTextureList();
	win->SetColor("mat_am", temp.ColorAmbient);
	win->SetColor("mat_di", temp.ColorDiffuse);
	win->SetColor("mat_sp", temp.ColorSpecular);
	win->SetColor("mat_em", temp.ColorEmissive);
	SetFloat("mat_shininess", temp.ColorShininess);

	if (temp.TransparencyMode == TransparencyModeColorKeySmooth)
		Check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeColorKeyHard)
		Check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeFactor)
		Check("transparency_mode:factor", true);
	else if (temp.TransparencyMode == TransparencyModeFunctions)
		Check("transparency_mode:function", true);
	else
		Check("transparency_mode:none", true);
	Enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	Enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	Enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	SetFloat("alpha_factor", temp.AlphaFactor * 100.0f);
	Check("alpha_z_buffer", temp.AlphaZBuffer);
	SetInt("alpha_source", temp.AlphaSource);
	SetInt("alpha_dest", temp.AlphaDestination);

	if (temp.ReflectionMode == ReflectionCubeMapStatic)
		Check("reflection_mode:cube_static", true);
	else if (temp.ReflectionMode == ReflectionCubeMapDynamical)
		Check("reflection_mode:cube_dynamic", true);
	else
		Check("reflection_mode:none", true);
	SetInt("reflection_size", temp.ReflectionSize);
	SetInt("reflection_density", temp.ReflectionDensity);
	RefillReflTexView();
	Enable("reflection_size", ((temp.ReflectionMode == ReflectionCubeMapStatic) || (temp.ReflectionMode == ReflectionCubeMapDynamical)));
	Enable("reflection_textures", (temp.ReflectionMode == ReflectionCubeMapStatic));
	Enable("reflection_density", (temp.ReflectionMode != ReflectionNone));
	SetString("shader_file", temp.ShaderFile);


    SetFloat("rcjump", temp_phys.RCJump);
    SetFloat("rcstatic", temp_phys.RCStatic);
    SetFloat("rcsliding", temp_phys.RCSliding);
    SetFloat("rcroll", temp_phys.RCRolling);
}

MaterialPropertiesDialog::~MaterialPropertiesDialog()
{
	Unsubscribe(data);
}

void MaterialPropertiesDialog::OnUpdate(Observable *o)
{
	temp = data->Appearance;
	temp_phys = data->Physics;
	LoadData();
}

void MaterialPropertiesDialog::OnAddTextureLevel()
{
	if (temp.NumTextureLevels >= MATERIAL_MAX_TEXTURES){
		ed->ErrorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.TextureFile[temp.NumTextureLevels ++] = "";
	ApplyData();
}

void MaterialPropertiesDialog::OnTextures()
{
	int sel = GetInt("");
	if ((sel >= 0) && (sel <temp.NumTextureLevels))
		if (ed->FileDialog(FDTexture, false, true)){
			temp.TextureFile[sel] = ed->DialogFile;
			ApplyData();
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			FillTextureList();
		}
}

void MaterialPropertiesDialog::OnTexturesSelect()
{
	int sel = GetInt("");
	Enable("mat_delete_texture_level", (sel >= 0) && (sel < temp.NumTextureLevels));
	Enable("mat_empty_texture_level", (sel >= 0) && (sel < temp.NumTextureLevels));
}

void MaterialPropertiesDialog::OnDeleteTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		for (int i=sel;i<temp.NumTextureLevels-1;i++)
			temp.TextureFile[i] = temp.TextureFile[i + 1];
		temp.NumTextureLevels --;
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnEmptyTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		temp.TextureFile[sel] = "";
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnTransparencyMode()
{
	if (IsChecked("transparency_mode:function"))
		temp.TransparencyMode = TransparencyModeFunctions;
	else if (IsChecked("transparency_mode:color_key"))
		temp.TransparencyMode = TransparencyModeColorKeyHard;
	else if (IsChecked("transparency_mode:factor"))
		temp.TransparencyMode = TransparencyModeFactor;
	else
		temp.TransparencyMode = TransparencyModeNone;
	Enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	Enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	Enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionMode()
{
	if (IsChecked("reflection_mode:cube_static"))
		temp.ReflectionMode = ReflectionCubeMapStatic;
	else if (IsChecked("reflection_mode:cube_dynamic"))
		temp.ReflectionMode = ReflectionCubeMapDynamical;
	else
		temp.ReflectionMode = ReflectionNone;
	Enable("reflection_size", ((temp.ReflectionMode == ReflectionCubeMapStatic) || (temp.ReflectionMode == ReflectionCubeMapDynamical)));
	Enable("reflection_textures", (temp.ReflectionMode == ReflectionCubeMapStatic));
	Enable("reflection_density", (temp.ReflectionMode != ReflectionNone));
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionTextures()
{
	int sel=GetInt("");
	if (ed->FileDialog(FDTexture,false,true)){
		temp.ReflectionTextureFile[sel] = ed->DialogFile;
		if ((sel==0)&&(temp.ReflectionTextureFile[0].find(".") >= 0)){
			int p=temp.ReflectionTextureFile[0].find(".");
			for (int i=1;i<6;i++){
				string tf;
				tf = temp.ReflectionTextureFile[0];
				tf[p-1]=temp.ReflectionTextureFile[0][p-1]+i;
				if (file_test_existence(NixTextureDir + tf)){
					temp.ReflectionTextureFile[i] = tf;
				}
			}

		}
		ApplyData();
		RefillReflTexView();
	}
}

bool TestShaderFile(const string &filename)
{
	int shader = NixLoadShader(filename);
	NixUnrefShader(shader);
	return shader >= 0;
}

void MaterialPropertiesDialog::OnFindShader()
{
	if (ed->FileDialog(FDShaderFile,false,true)){
		if (TestShaderFile(ed->DialogFile)){
			SetString("shader_file", ed->DialogFile);
			ApplyData();
		}else{
			ed->ErrorBox(_("Fehler in der Shader-Datei:\n") + NixShaderError);
		}
	}
}

void MaterialPropertiesDialog::ApplyData()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	temp.ColorAmbient = win->GetColor("mat_am");
	temp.ColorDiffuse = win->GetColor("mat_di");
	temp.ColorSpecular = win->GetColor("mat_sp");
	temp.ColorEmissive = win->GetColor("mat_em");
	temp.ColorShininess = GetFloat("mat_shininess");
	temp.AlphaZBuffer = IsChecked("alpha_z_buffer");
	temp.AlphaFactor = GetFloat("alpha_factor") * 0.01f;
	temp.AlphaSource = GetInt("alpha_source");
	temp.AlphaDestination = GetInt("alpha_dest");

	temp.ReflectionDensity = GetInt("reflection_density");
	temp.ReflectionSize = GetInt("reflection_size");

	temp.ShaderFile = GetString("shader_file");

	data->Execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::ApplyDataDelayed()
{
	apply_queue_depth ++;
	HuiRunLaterM(0.5f, this, &MaterialPropertiesDialog::ApplyData);
}

void MaterialPropertiesDialog::ApplyPhysData()
{
	if (apply_phys_queue_depth> 0)
		apply_phys_queue_depth --;
	if (apply_phys_queue_depth > 0)
		return;
	temp_phys.RCJump = GetFloat("rcjump");
	temp_phys.RCStatic = GetFloat("rcstatic");
	temp_phys.RCSliding = GetFloat("rcsliding");
	temp_phys.RCRolling = GetFloat("rcroll");

	data->Execute(new ActionMaterialEditPhysics(temp_phys));
}

void MaterialPropertiesDialog::ApplyPhysDataDelayed()
{
	apply_phys_queue_depth ++;
	HuiRunLaterM(0.5f, this, &MaterialPropertiesDialog::ApplyPhysData);
}

void MaterialPropertiesDialog::RefillReflTexView()
{
	Reset("reflection_textures");
	AddString("reflection_textures", " + X\\" + temp.ReflectionTextureFile[0]);
	AddString("reflection_textures", " - X\\" + temp.ReflectionTextureFile[1]);
	AddString("reflection_textures", " + Y\\" + temp.ReflectionTextureFile[2]);
	AddString("reflection_textures", " - Y\\" + temp.ReflectionTextureFile[3]);
	AddString("reflection_textures", " + Z\\" + temp.ReflectionTextureFile[4]);
	AddString("reflection_textures", " - Z\\" + temp.ReflectionTextureFile[5]);
}

void MaterialPropertiesDialog::FillTextureList()
{
	Reset("mat_textures");
	for (int i=0;i<temp.NumTextureLevels;i++){
		int tex = NixLoadTexture(temp.TextureFile[i]);
		string img = ed->get_tex_image(tex);
		AddString("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	if (temp.NumTextureLevels == 0)
		AddString("mat_textures", _("\\\\   - keine Texturen -"));
	Enable("mat_delete_texture_level", false);
	Enable("mat_empty_texture_level", false);
}
