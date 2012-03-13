/*
 * MaterialPropertiesDialog.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "MaterialPropertiesDialog.h"
#include "../../../Edward.h"
#include "../ModeMaterial.h"



MaterialPropertiesDialog::MaterialPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataMaterial *_data):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("material_dialog");
	EventM("cancel", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnClose);
	EventM("hui:close", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnClose);
	EventM("set", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::ApplyData);
	EventM("ok", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnOk);
	EventM("mat_add_texture_level", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnMatAddTextureLevel);
	EventM("mat_textures", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnMatTextures);
	EventM("mat_delete_texture_level", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnMatDeleteTextureLevel);
	EventM("mat_empty_texture_level", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnMatEmptyTextureLevel);
	EventM("transparency_mode", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnTransparencyMode);
	EventM("reflection", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnReflection);
	EventM("reflection_textures", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnReflectionTextures);
	EventM("find_effect", this, (void(CHuiWindow::*)())&MaterialPropertiesDialog::OnFindEffect);

	LoadData();
}

void MaterialPropertiesDialog::LoadData()
{
	TempNumTextureLevels = data->NumTextureLevels;
	for (int i=0;i<data->NumTextureLevels;i++)
		TempTextureFile[i] = data->TextureFile[i];
	FillTextureList();
	SetColor("mat_am", data->ColorAmbient);
	SetColor("mat_di", data->ColorDiffuse);
	SetColor("mat_sp", data->ColorSpecular);
	SetColor("mat_em", data->ColorEmissive);
	SetInt("mat_shininess", data->ColorShininess);
	if (data->TransparencyMode==TransparencyModeColorKeySmooth)
		SetInt("transparency_mode",1);
	else if (data->TransparencyMode==TransparencyModeColorKeyHard)
		SetInt("transparency_mode",2);
	else if(data->TransparencyMode==TransparencyModeFactor)
		SetInt("transparency_mode",3);
	else if(data->TransparencyMode==TransparencyModeFunctions)
		SetInt("transparency_mode",4);
	else
		SetInt("transparency_mode",0);
	Enable("alpha_factor", data->TransparencyMode==TransparencyModeFactor);
	Enable("alpha_source", data->TransparencyMode==TransparencyModeFunctions);
	Enable("alpha_dest", data->TransparencyMode==TransparencyModeFunctions);
	SetInt("alpha_factor", data->AlphaFactor);
	Check("alpha_z_buffer", data->AlphaZBuffer);
	SetInt("alpha_source", data->AlphaSource);
	SetInt("alpha_dest", data->AlphaDestination);

	SetInt("reflection", data->ReflectionMode);
	SetInt("reflection_size", data->ReflectionSize);
	SetInt("reflection_density", data->ReflectionDensity);
	for (int i=0;i<6;i++)
		ReflTex[i] = data->ReflectionTextureFile[i];
	RefillReflTexView();
	Enable("reflection_size", ((data->ReflectionMode == ReflectionCubeMapStatic) || (data->ReflectionMode == ReflectionCubeMapDynamical)));
	Enable("reflection_textures", (data->ReflectionMode == ReflectionCubeMapStatic));
	Enable("reflection_density", (data->ReflectionMode != ReflectionNone));
	SetInt("shining", data->ShiningDensity);
	SetInt("shining_length", data->ShiningLength);
	SetString("effect_file", data->EffectFile);
	Check("water", data->Water);
}

MaterialPropertiesDialog::~MaterialPropertiesDialog()
{
}

void MaterialPropertiesDialog::OnMatAddTextureLevel()
{
	if (TempNumTextureLevels >= MATERIAL_MAX_TEXTURE_LEVELS)
		ed->ErrorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURE_LEVELS));
	else
		TempTextureFile[TempNumTextureLevels ++] = "";
	FillTextureList();
}

void MaterialPropertiesDialog::OnMatTextures()
{
	int sel = GetInt("");
	if ((sel >= 0) && (sel <TempNumTextureLevels))
		if (ed->FileDialog(FDTexture, false, true)){
			TempTextureFile[sel] = ed->DialogFile;
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			FillTextureList();
		}
}

void MaterialPropertiesDialog::OnMatDeleteTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		for (int i=sel;i<TempNumTextureLevels-1;i++)
			TempTextureFile[i] = TempTextureFile[i + 1];
		TempNumTextureLevels --;
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnMatEmptyTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		TempTextureFile[sel] = "";
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
		ReflTex[sel] = ed->DialogFile;
		if ((sel==0)&&(ReflTex[0].find(".") >= 0)){
			int p=ReflTex[0].find(".");
			for (int i=1;i<6;i++){
				string tf;
				tf = ReflTex[0];
				tf[p-1]=ReflTex[0][p-1]+i;
				if (file_test_existence(NixTextureDir + tf)){
					ReflTex[i] = tf;
				}
			}

		}
	//	mode_material->RefillReflTexView();
	// TODO
	}
}

void MaterialPropertiesDialog::OnFindEffect()
{
	if (ed->FileDialog(FDShaderFile,false,true))
		SetString("effect_file", ed->DialogFileNoEnding);
}

void MaterialPropertiesDialog::ApplyData()
{
	for (int i=0;i<TempNumTextureLevels;i++){
		if (i < data->NumTextureLevels)
			if (data->TextureFile[i] != TempTextureFile[i])
				data->Texture[i] = NixLoadTexture(TempTextureFile[i]);
		data->TextureFile[i] = TempTextureFile[i];
	}
	data->NumTextureLevels = TempNumTextureLevels;
	data->ColorAmbient = GetColor("mat_am");
	data->ColorDiffuse = GetColor("mat_di");
	data->ColorSpecular = GetColor("mat_sp");
	data->ColorEmissive = GetColor("mat_em");
	data->ColorShininess = GetInt("mat_shininess");
	if (GetInt("transparency_mode")==1)
		data->TransparencyMode=TransparencyModeColorKeySmooth;
	else if (GetInt("transparency_mode")==2)
		data->TransparencyMode=TransparencyModeColorKeyHard;
	else if (GetInt("transparency_mode")==3)
		data->TransparencyMode=TransparencyModeFactor;
	else if (GetInt("transparency_mode")==4)
		data->TransparencyMode=TransparencyModeFunctions;
	else
		data->TransparencyMode=TransparencyModeNone;
	data->AlphaZBuffer=IsChecked("alpha_z_buffer");
	data->AlphaFactor=GetInt("alpha_factor");
	data->AlphaSource=GetInt("alpha_source");
	data->AlphaDestination=GetInt("alpha_dest");
	data->ShiningDensity=GetInt("shining");
	data->ShiningLength=GetInt("shining_length");
	data->Water=IsChecked("water");

	data->ReflectionMode = GetInt("reflection");
	data->ReflectionDensity = GetInt("reflection_density");
	data->ReflectionSize = GetInt("reflection_size");
	for (int i=0;i<6;i++)
		data->ReflectionTextureFile[i] = ReflTex[i];

	if (data->EffectFile != GetString("effect_file")){
		NixDeleteShader(data->EffectIndex);
		data->EffectFile = GetString("effect_file");

		data->EffectIndex = NixLoadShader(MaterialDir + data->EffectFile + ".fx");
	}
	data->Notify("Change");
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
	AddString("reflection_textures", " + X\\" + ReflTex[0]);
	AddString("reflection_textures", " - X\\" + ReflTex[1]);
	AddString("reflection_textures", " + Y\\" + ReflTex[2]);
	AddString("reflection_textures", " - Y\\" + ReflTex[3]);
	AddString("reflection_textures", " + Z\\" + ReflTex[4]);
	AddString("reflection_textures", " - Z\\" + ReflTex[5]);
}

void MaterialPropertiesDialog::FillTextureList()
{
	Reset("mat_textures");
	for (int i=0;i<TempNumTextureLevels;i++){
		string img = ed->get_tex_image(NixLoadTexture(TempTextureFile[i]));
		if (TempTextureFile[i].num > 0)
			AddString("mat_textures", format("%d\\%s\\%s", i, img.c_str(), TempTextureFile[i].c_str()));
		else
			AddString("mat_textures", format("%d\\\\%s", i, _("   - ohne Datei -").c_str()));
	}
	if (TempNumTextureLevels == 0)
		AddString("mat_textures", _("\\\\   - keine Texturen -"));
	Enable("mat_delete_texture_level", TempNumTextureLevels > 0);
}
