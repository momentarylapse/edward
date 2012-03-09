/*
 * MaterialPropertiesDialog.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "MaterialPropertiesDialog.h"
#include "../../../Edward.h"



#if 0
void OnAppMatAddTextureLevel()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	if (TempNumTextureLevels >= MATERIAL_MAX_TEXTURE_LEVELS)
		CModeAll::ErrorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURE_LEVELS));
	else
		TempTextureFile[TempNumTextureLevels ++] = "";
	FillTextureList();
}

void OnAppMatTextures()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	int sel = dlg->GetInt("");
	if ((sel >= 0) && (sel <TempNumTextureLevels))
		if (FileDialog(FDTexture, false, true)){
			TempTextureFile[sel] = DialogFile;
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			FillTextureList();
		}
}

void OnAppMatDeleteTextureLevel()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	int sel = dlg->GetInt("mat_textures");
	if (sel >= 0){
		for (int i=sel;i<TempNumTextureLevels-1;i++)
			TempTextureFile[i] = TempTextureFile[i + 1];
		TempNumTextureLevels --;
		FillTextureList();
	}
}

void OnAppMatEmptyTextureLevel()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	int sel = dlg->GetInt("mat_textures");
	if (sel >= 0){
		TempTextureFile[sel] = "";
		FillTextureList();
	}
}

void OnAppTransparencyMode()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	int sel = dlg->GetInt("");
	dlg->Enable("alpha_factor",sel==3);
	dlg->Enable("alpha_source",sel==4);
	dlg->Enable("alpha_dest",sel==4);
	dlg->Check("alpha_z_buffer",(sel!=3)&&(sel!=4));
}

void OnAppReflection()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	int sel=dlg->GetInt("");
	dlg->Enable("reflection_size",((sel==ReflectionCubeMapStatic)||(sel==ReflectionCubeMapDynamical)));
	dlg->Enable("reflection_textures",(sel==ReflectionCubeMapStatic));
	dlg->Enable("reflection_density",(sel!=ReflectionNone));
}

void OnAppReflectionTextures()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	int sel=dlg->GetInt("");
	if (FileDialog(FDTexture,false,true)){
		ReflTex[sel] = DialogFile;
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
		mmaterial->RefillReflTexView();
	}
}

void OnAppFindEffect()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	if (FileDialog(FDShaderFile,false,true))
		dlg->SetString("effect_file",DialogFileNoEnding);
}

void OnAppSet()
{
	CHuiWindow *dlg = mmaterial->AppearanceDialog;
	for (int i=0;i<TempNumTextureLevels;i++){
		if (i < mmaterial->NumTextureLevels)
			if (mmaterial->TextureFile[i] != TempTextureFile[i])
				mmaterial->Texture[i] = NixLoadTexture(TempTextureFile[i]);
		mmaterial->TextureFile[i] = TempTextureFile[i];
	}
	mmaterial->NumTextureLevels = TempNumTextureLevels;
	mmaterial->ColorAmbient = dlg->GetColor("mat_am");
	mmaterial->ColorDiffuse = dlg->GetColor("mat_di");
	mmaterial->ColorSpecular = dlg->GetColor("mat_sp");
	mmaterial->ColorEmissive = dlg->GetColor("mat_em");
	mmaterial->ColorShininess = dlg->GetInt("mat_shininess");
	if (dlg->GetInt("transparency_mode")==1)
		mmaterial->TransparencyMode=TransparencyModeColorKeySmooth;
	else if (dlg->GetInt("transparency_mode")==2)
		mmaterial->TransparencyMode=TransparencyModeColorKeyHard;
	else if (dlg->GetInt("transparency_mode")==3)
		mmaterial->TransparencyMode=TransparencyModeFactor;
	else if (dlg->GetInt("transparency_mode")==4)
		mmaterial->TransparencyMode=TransparencyModeFunctions;
	else
		mmaterial->TransparencyMode=TransparencyModeNone;
	mmaterial->AlphaZBuffer=dlg->IsChecked("alpha_z_buffer");
	mmaterial->AlphaFactor=dlg->GetInt("alpha_factor");
	mmaterial->AlphaSource=dlg->GetInt("alpha_source");
	mmaterial->AlphaDestination=dlg->GetInt("alpha_dest");
	mmaterial->ShiningDensity=dlg->GetInt("shining");
	mmaterial->ShiningLength=dlg->GetInt("shining_length");
	mmaterial->Water=dlg->IsChecked("water");

	mmaterial->ReflectionMode=dlg->GetInt("reflection");
	mmaterial->ReflectionDensity=dlg->GetInt("reflection_density");
	mmaterial->ReflectionSize=dlg->GetInt("reflection_size");
	for (int i=0;i<6;i++)
		mmaterial->ReflectionTextureFile[i] = ReflTex[i];

	if (mmaterial->EffectFile != dlg->GetString("effect_file")){
		NixDeleteShader(mmaterial->EffectIndex);
		mmaterial->EffectFile = dlg->GetString("effect_file");

		mmaterial->EffectIndex = NixLoadShader(MaterialDir + mmaterial->EffectFile + ".fx");
	}
	Change();
}

void OnAppOk()
{
	OnAppSet();
	delete(mmaterial->AppearanceDialog);
	mmaterial->AppearanceDialog=NULL;
}

void OnAppClose()
{
	delete(mmaterial->AppearanceDialog);
	mmaterial->AppearanceDialog=NULL;
}
#endif

MaterialPropertiesDialog::MaterialPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataMaterial *_data):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("material_dialog");
	/*Event("cancel", &OnAppClose);
	Event("hui:close", &OnAppClose);
	Event("set", &OnAppSet);
	Event("ok", &OnAppOk);
	Event("mat_add_texture_level", &OnAppMatAddTextureLevel);
	Event("mat_textures", &OnAppMatTextures);
	Event("mat_delete_texture_level", &OnAppMatDeleteTextureLevel);
	Event("mat_empty_texture_level", &OnAppMatEmptyTextureLevel);
	Event("transparency_mode", &OnAppTransparencyMode);
	Event("reflection", &OnAppReflection);
	Event("reflection_textures", &OnAppReflectionTextures);
	Event("find_effect", &OnAppFindEffect);

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
		SetInt("mat_transparency_mode",1);
	else if (data->TransparencyMode==TransparencyModeColorKeyHard)
		SetInt("mat_transparency_mode",2);
	else if(data->TransparencyMode==TransparencyModeFactor)
		SetInt("mat_transparency_mode",3);
	else if(data->TransparencyMode==TransparencyModeFunctions)
		SetInt("mat_transparency_mode",4);
	else
		SetInt("mat_transparency_mode",0);
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
	Check("water", data->Water);*/
}

MaterialPropertiesDialog::~MaterialPropertiesDialog()
{
}
