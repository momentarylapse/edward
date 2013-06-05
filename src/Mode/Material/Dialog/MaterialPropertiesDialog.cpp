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


string file_secure(const string &filename); // -> ModelPropertiesDialog

extern string NixShaderError; // -> nix

MaterialPropertiesDialog::MaterialPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataMaterial *_data):
	HuiWindow("material_dialog", _parent, _allow_parent)
{
	data = _data;

	// dialog
	EventM("cancel", this, &MaterialPropertiesDialog::OnClose);
	EventM("hui:close", this, &MaterialPropertiesDialog::OnClose);
	EventM("set", this, &MaterialPropertiesDialog::ApplyData);
	EventM("ok", this, &MaterialPropertiesDialog::OnOk);
	EventM("mat_add_texture_level", this, &MaterialPropertiesDialog::OnAddTextureLevel);
	EventM("mat_textures", this, &MaterialPropertiesDialog::OnTextures);
	EventMX("mat_textures", "hui:select", this, &MaterialPropertiesDialog::OnTexturesSelect);
	EventM("mat_delete_texture_level", this, &MaterialPropertiesDialog::OnDeleteTextureLevel);
	EventM("mat_empty_texture_level", this, &MaterialPropertiesDialog::OnEmptyTextureLevel);
	EventM("transparency_mode", this, &MaterialPropertiesDialog::OnTransparencyMode);
	EventM("reflection", this, &MaterialPropertiesDialog::OnReflection);
	EventM("reflection_textures", this, &MaterialPropertiesDialog::OnReflectionTextures);
	EventM("find_shader", this, &MaterialPropertiesDialog::OnFindShader);

	temp = data->Appearance;
	LoadData();
	Subscribe(data);
}

void MaterialPropertiesDialog::LoadData()
{
	FillTextureList();
	SetColor("mat_am", temp.ColorAmbient);
	SetColor("mat_di", temp.ColorDiffuse);
	SetColor("mat_sp", temp.ColorSpecular);
	SetColor("mat_em", temp.ColorEmissive);
	SetFloat("mat_shininess", temp.ColorShininess);
	if (temp.TransparencyMode==TransparencyModeColorKeySmooth)
		SetInt("transparency_mode",1);
	else if (temp.TransparencyMode==TransparencyModeColorKeyHard)
		SetInt("transparency_mode",2);
	else if(temp.TransparencyMode==TransparencyModeFactor)
		SetInt("transparency_mode",3);
	else if(temp.TransparencyMode==TransparencyModeFunctions)
		SetInt("transparency_mode",4);
	else
		SetInt("transparency_mode",0);
	Enable("alpha_factor", temp.TransparencyMode==TransparencyModeFactor);
	Enable("alpha_source", temp.TransparencyMode==TransparencyModeFunctions);
	Enable("alpha_dest", temp.TransparencyMode==TransparencyModeFunctions);
	SetFloat("alpha_factor", temp.AlphaFactor * 100.0f);
	Check("alpha_z_buffer", temp.AlphaZBuffer);
	SetInt("alpha_source", temp.AlphaSource);
	SetInt("alpha_dest", temp.AlphaDestination);

	SetInt("reflection", temp.ReflectionMode);
	SetInt("reflection_size", temp.ReflectionSize);
	SetInt("reflection_density", temp.ReflectionDensity);
	RefillReflTexView();
	Enable("reflection_size", ((temp.ReflectionMode == ReflectionCubeMapStatic) || (temp.ReflectionMode == ReflectionCubeMapDynamical)));
	Enable("reflection_textures", (temp.ReflectionMode == ReflectionCubeMapStatic));
	Enable("reflection_density", (temp.ReflectionMode != ReflectionNone));
	SetString("shader_file", temp.ShaderFile);
}

MaterialPropertiesDialog::~MaterialPropertiesDialog()
{
	Unsubscribe(data);
}

void MaterialPropertiesDialog::OnUpdate(Observable *o)
{
	temp = data->Appearance;
	LoadData();
}

void MaterialPropertiesDialog::OnAddTextureLevel()
{
	if (temp.NumTextureLevels >= MATERIAL_MAX_TEXTURES)
		ed->ErrorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
	else
		temp.TextureFile[temp.NumTextureLevels ++] = "";
	FillTextureList();
}

void MaterialPropertiesDialog::OnTextures()
{
	int sel = GetInt("");
	if ((sel >= 0) && (sel <temp.NumTextureLevels))
		if (ed->FileDialog(FDTexture, false, true)){
			temp.TextureFile[sel] = ed->DialogFile;
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
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnEmptyTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		temp.TextureFile[sel] = "";
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnTransparencyMode()
{
	int sel = GetInt("");
	Enable("alpha_factor",sel==3);
	Enable("alpha_source",sel==4);
	Enable("alpha_dest",sel==4);
	Check("alpha_z_buffer",(sel!=3)&&(sel!=4));
}

void MaterialPropertiesDialog::OnReflection()
{
	int sel=GetInt("");
	Enable("reflection_size",((sel==ReflectionCubeMapStatic)||(sel==ReflectionCubeMapDynamical)));
	Enable("reflection_textures",(sel==ReflectionCubeMapStatic));
	Enable("reflection_density",(sel!=ReflectionNone));
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
		}else{
			ed->ErrorBox(_("Fehler in der Shader-Datei:\n") + NixShaderError);
		}
	}
}

void MaterialPropertiesDialog::ApplyData()
{
	temp.ColorAmbient = GetColor("mat_am");
	temp.ColorDiffuse = GetColor("mat_di");
	temp.ColorSpecular = GetColor("mat_sp");
	temp.ColorEmissive = GetColor("mat_em");
	temp.ColorShininess = GetFloat("mat_shininess");
	int n = GetInt("transparency_mode");
	if (n == 1)
		temp.TransparencyMode = TransparencyModeColorKeySmooth;
	else if (n == 2)
		temp.TransparencyMode = TransparencyModeColorKeyHard;
	else if (n == 3)
		temp.TransparencyMode = TransparencyModeFactor;
	else if (n == 4)
		temp.TransparencyMode = TransparencyModeFunctions;
	else
		temp.TransparencyMode = TransparencyModeNone;
	temp.AlphaZBuffer = IsChecked("alpha_z_buffer");
	temp.AlphaFactor = GetFloat("alpha_factor") * 0.01f;
	temp.AlphaSource = GetInt("alpha_source");
	temp.AlphaDestination = GetInt("alpha_dest");

	temp.ReflectionMode = GetInt("reflection");
	temp.ReflectionDensity = GetInt("reflection_density");
	temp.ReflectionSize = GetInt("reflection_size");


	temp.ShaderFile = GetString("shader_file");

	data->Execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::OnOk()
{
	ApplyData();
	delete(this);
	mode_material->AppearanceDialog = NULL;
}

void MaterialPropertiesDialog::OnClose()
{
	delete(this);
	mode_material->AppearanceDialog = NULL;
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
		AddString("mat_textures", format("%d\\%s\\%s", i, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	if (temp.NumTextureLevels == 0)
		AddString("mat_textures", _("\\\\   - keine Texturen -"));
	Enable("mat_delete_texture_level", false);
	Enable("mat_empty_texture_level", false);
}
