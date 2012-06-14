/*
 * ModelMaterialDialog.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ModelMaterialDialog.h"
#include "../ModeModel.h"
#include "../../../Edward.h"
#include "../../../Action/Model/Data/ActionModelEditMaterial.h"



string file_secure(const string &filename); // -> ModelPropertiesDialog

ModelMaterialDialog::ModelMaterialDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("model_material_dialog");

	EventM("cancel", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnClose);
	EventM("set", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::ApplyData);
	EventM("ok", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnOk);
	EventM("transparency_mode", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnTransparencyMode);
	EventM("default_transparency", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnDefaultTransparency);
	EventM("mat_add_texture_level", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnMatAddTextureLevel);
	EventMX("mat_textures", "hui:activate", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnMatTextures);
	EventMX("mat_textures", "hui:select", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnMatTexturesSelect);
	EventM("mat_delete_texture_level", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnMatDeleteTextureLevel);
	EventM("mat_empty_texture_level", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnMatEmptyTextureLevel);
	EventM("default_material", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnDefaultMaterial);
	EventM("find_material", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnFindMaterial);
	EventM("edit_material", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnEditMaterial);
	EventM("default_colors", this, (void(HuiEventHandler::*)())&ModelMaterialDialog::OnDefaultColors);

	index = data->CurrentMaterial;
	TempMaterial = data->Material[index];
	LoadData();

	Subscribe(data);
}

ModelMaterialDialog::~ModelMaterialDialog()
{
	mode_model->MaterialDialog = NULL;
	Unsubscribe(data);
}

void ModelMaterialDialog::LoadData()
{
	ModeModelMaterial *mat = &TempMaterial;
	// material
	Check("default_colors", !mat->UserColor);
	Enable("mat_am", mat->UserColor);
	Enable("mat_di", mat->UserColor);
	Enable("mat_sp", mat->UserColor);
	Enable("mat_em", mat->UserColor);
	Enable("mat_shininess", mat->UserColor);
	SetColor("mat_am", mat->Ambient);
	SetColor("mat_di", mat->Diffuse);
	SetColor("mat_sp", mat->Specular);
	SetColor("mat_em", mat->Emission);
	SetInt("mat_shininess", mat->Shininess);
	SetString("material_file", mat->MaterialFile);
	Check("default_material", mat->MaterialFile.num == 0);
	Enable("material_file", mat->MaterialFile.num > 0);
	// transparency
	Check("default_transparency", !mat->UserTransparency);
	if (mat->TransparencyMode==TransparencyModeColorKeySmooth)
		SetInt("transparency_mode",1);
	else if (mat->TransparencyMode==TransparencyModeColorKeyHard)
		SetInt("transparency_mode",2);
	else if (mat->TransparencyMode==TransparencyModeFactor)
		SetInt("transparency_mode",3);
	else if (mat->TransparencyMode==TransparencyModeFunctions)
		SetInt("transparency_mode",4);
	else
		SetInt("transparency_mode",0);
	Enable("transparency_mode", mat->UserTransparency);
	Enable("alpha_factor", (mat->TransparencyMode==TransparencyModeFactor) && mat->UserTransparency);
	Enable("alpha_source", (mat->TransparencyMode==TransparencyModeFunctions) && mat->UserTransparency);
	Enable("alpha_dest", (mat->TransparencyMode==TransparencyModeFunctions) && mat->UserTransparency);
	Enable("alpha_z_buffer", (mat->TransparencyMode==TransparencyModeFactor) && mat->UserTransparency);
	SetInt("alpha_source", mat->AlphaSource);
	SetInt("alpha_dest", mat->AlphaDestination);
	Check("alpha_z_buffer", mat->AlphaZBuffer);
	SetFloat("alpha_factor", mat->AlphaFactor * 100.0f);

	/*if (for_creation)
		Enable("apply", false);*/
	FillTextureList();
}


void ModelMaterialDialog::FillTextureList()
{
	Reset("mat_textures");
	for (int i=0;i<TempMaterial.NumTextures;i++){
		string img = ed->get_tex_image(TempMaterial.Texture[i]);
		AddString("mat_textures", i2s(i) + "\\" + img + "\\" + file_secure(TempMaterial.TextureFile[i]));
	}

	Enable("mat_delete_texture_level", false);
	Enable("mat_empty_texture_level", false);
}

// transparency
void ModelMaterialDialog::OnTransparencyMode()
{
	int sel = GetInt("");
	Enable("alpha_factor",sel==3);
	Enable("alpha_source",sel==4);
	Enable("alpha_dest",sel==4);
	Check("alpha_z_buffer",(sel!=3)&&(sel!=4));
}

void ModelMaterialDialog::OnDefaultTransparency()
{
	TempMaterial.UserTransparency = !IsChecked("");
	if (!TempMaterial.UserTransparency)
		TempMaterial.CheckTransparency();
	LoadData();
}

// textures
void ModelMaterialDialog::OnMatAddTextureLevel()
{
	if (TempMaterial.NumTextures >= MODEL_MAX_TEXTURES){
		ed->ErrorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MODEL_MAX_TEXTURES));
		return;
	}
	TempMaterial.TextureFile[TempMaterial.NumTextures ++] = "";
	TempMaterial.CheckTextures();
	FillTextureList();
}


void ModelMaterialDialog::OnMatTextures()
{
	int sel = GetInt("");
	if ((sel >= 0) && (sel < TempMaterial.NumTextures))
		if (ed->FileDialog(FDTexture, false, true)){
			TempMaterial.TextureFile[sel] = ed->DialogFile;
			TempMaterial.CheckTextures();
			FillTextureList();
		}
}


void ModelMaterialDialog::OnMatTexturesSelect()
{
	int sel = GetInt("");
	Enable("mat_delete_texture_level", sel >= 0);
	Enable("mat_empty_texture_level", sel >= 0);
}

void ModelMaterialDialog::OnMatDeleteTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		if (TempMaterial.NumTextures <= 1){
			ed->ErrorBox(_("Mindestens eine Textur-Ebene muss vorhanden sein!"));
			return;
		}
		for (int i=sel;i<TempMaterial.NumTextures-1;i++)
			TempMaterial.TextureFile[i] = TempMaterial.TextureFile[i + 1];
		TempMaterial.NumTextures --;
		TempMaterial.CheckTextures();
		FillTextureList();
	}
}

void ModelMaterialDialog::OnMatEmptyTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		TempMaterial.TextureFile[sel] = "";
		TempMaterial.CheckTextures();
		FillTextureList();
	}
}

// material
void ModelMaterialDialog::OnDefaultMaterial()
{
	if (IsChecked(""))
		TempMaterial.MaterialFile = "";
	TempMaterial.MakeConsistent();
	LoadData();
}

		/*case HMM_MATERIAL_FILE:
			dlg->Enable(HMM_MATERIAL_FILE,strlen(MaterialFile)>0);
			break;*/

void ModelMaterialDialog::OnFindMaterial()
{
	if (ed->FileDialog(FDMaterial, false, true)){
		TempMaterial.MaterialFile = ed->DialogFileNoEnding;
		TempMaterial.MakeConsistent();
		LoadData();
	}
}

void ModelMaterialDialog::OnEditMaterial()
{
	/*if (GetString("material_file").num > 0)
		if (mode_material->LoadFromFile(MaterialDir + GetString("material_file") + ".material")){
			ed->SetMode(mode_material);
			OnClose();
		}*/
}

void ModelMaterialDialog::OnDefaultColors()
{
	TempMaterial.UserColor = !IsChecked("");
	if (!TempMaterial.UserColor)
		TempMaterial.CheckColors();
	LoadData();
}


void ModelMaterialDialog::ApplyData()
{
// material
	if (IsChecked("default_material"))
		TempMaterial.MaterialFile = "";
	else
		TempMaterial.MaterialFile = GetString("material_file");
	TempMaterial.Ambient = GetColor("mat_am");
	TempMaterial.Diffuse = GetColor("mat_di");
	TempMaterial.Specular = GetColor("mat_sp");
	TempMaterial.Emission = GetColor("mat_em");
	TempMaterial.Shininess = GetInt("mat_shininess");
	TempMaterial.UserColor = !IsChecked("default_colors");
	TempMaterial.material = MetaLoadMaterial(TempMaterial.MaterialFile);
// transparency
	int s = GetInt("transparency_mode");
	if (s==1)
		TempMaterial.TransparencyMode=TransparencyModeColorKeySmooth;
	else if (s==2)
		TempMaterial.TransparencyMode=TransparencyModeColorKeyHard;
	else if (s==3)
		TempMaterial.TransparencyMode=TransparencyModeFactor;
	else if (s==4)
		TempMaterial.TransparencyMode=TransparencyModeFunctions;
	else
		TempMaterial.TransparencyMode=TransparencyModeNone;
	if (IsChecked("default_transparency"))
		TempMaterial.TransparencyMode=TransparencyModeDefault;
	TempMaterial.AlphaFactor = GetFloat("alpha_factor") * 0.01f;
	TempMaterial.AlphaSource = GetInt("alpha_source");
	TempMaterial.AlphaDestination = GetInt("alpha_dest");
	TempMaterial.AlphaZBuffer = IsChecked("alpha_z_buffer");
// textures
	for (int i=0;i<TempMaterial.NumTextures;i++)
		TempMaterial.Texture[i] = NixLoadTexture(TempMaterial.TextureFile[i]);
			/*if (m->NumTextures < skin->Sub[mmodel->CurrentMaterial].NumTextures){
				for (int i=0;i<4;i++)
					mmodel->Skin[i].Sub[mmodel->CurrentMaterial].NumTextures = m->NumTextures;
			}else if (m->NumTextures > skin->Sub[mmodel->CurrentMaterial].NumTextures){
				for (int i=0;i<4;i++)
					mmodel->Skin[i].Sub[mmodel->CurrentMaterial].NumTextures = m->NumTextures;
				// TODO ...new SkinVertices...
			}*/

	TempMaterial.CheckTextures();

	data->Execute(new ActionModelEditMaterial(index, TempMaterial));
}

void ModelMaterialDialog::OnOk()
{
	ApplyData();
	delete(this);
}

void ModelMaterialDialog::OnClose()
{
	delete(this);
}

void ModelMaterialDialog::OnUpdate(Observable *o)
{
	if (index >= data->Material.num){
		delete(this);
		return;
	}

	// undo/redo -> show current state
	TempMaterial = data->Material[index];
	LoadData();
	// ...well... 3d editing also restores old state...
}
