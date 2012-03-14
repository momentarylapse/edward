/*
 * ModelMaterialDialog.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ModelMaterialDialog.h"
#include "../ModeModel.h"
#include "../../../Edward.h"



//color mat_get_col(ModeModelMaterial *m); // -> ModelPropertiesDialog
string file_secure(const string &filename);

ModelMaterialDialog::ModelMaterialDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("model_material_dialog");

	EventM("cancel", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnClose);
	EventM("hui:close", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnClose);
	EventM("set", this, (void(CHuiWindow::*)())&ModelMaterialDialog::ApplyData);
	EventM("ok", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnOk);
	EventM("transparency_mode", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnTransparencyMode);
	EventM("default_transparency", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnDefaultTransparency);
	EventM("mat_add_texture_level", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnMatAddTextureLevel);
	EventM("mat_textures", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnMatTextures);
	EventM("mat_delete_texture_level", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnMatDeleteTextureLevel);
	EventM("mat_empty_texture_level", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnMatEmptyTextureLevel);
	EventM("default_material", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnDefaultMaterial);
	EventM("find_material", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnFindMaterial);
	EventM("edit_material", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnEditMaterial);
	EventM("default_colors", this, (void(CHuiWindow::*)())&ModelMaterialDialog::OnDefaultColors);

	mat = &data->Material[data->CurrentMaterial];
	LoadData();
}

ModelMaterialDialog::~ModelMaterialDialog()
{
	mode_model->MaterialDialog = NULL;
}

void ModelMaterialDialog::LoadData()
{
	// material
	Check("default_colors", !mat->UserColor);
	Enable("mat_am", mat->UserColor);
	Enable("mat_di", mat->UserColor);
	Enable("mat_sp", mat->UserColor);
	Enable("mat_em", mat->UserColor);
	Enable("mat_shininess", mat->UserColor);
	SetColor("mat_am", mat->Color[0]);
	SetColor("mat_di", mat->Color[1]);
	SetColor("mat_sp", mat->Color[2]);
	SetColor("mat_em", mat->Color[3]);
	SetInt("mat_shininess", mat->Shininess);
	SetString("material_file", mat->MaterialFile);
	Check("default_material", mat->MaterialFile.num == 0);
	Enable("material_file", mat->MaterialFile.num > 0);
	// transparency
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
	if (mat->TransparencyMode<0){
		Check("default_transparency",true);
		Enable("transparency_mode",false);
		Enable("alpha_source",false);
		Enable("alpha_dest",false);
		Enable("alpha_factor",false);
		Enable("alpha_z_buffer",false);
	}else{
		Enable("alpha_factor", (mat->TransparencyMode==TransparencyModeFactor));
		Enable("alpha_source", (mat->TransparencyMode==TransparencyModeFunctions));
		Enable("alpha_dest", (mat->TransparencyMode==TransparencyModeFunctions));
		Enable("alpha_z_buffer", (mat->TransparencyMode==TransparencyModeFactor));
	}
	SetInt("alpha_source", mat->AlphaSource);
	SetInt("alpha_dest", mat->AlphaDestination);
	Check("alpha_z_buffer", mat->AlphaZBuffer);
	SetInt("alpha_factor", mat->AlphaFactor);

	/*if (for_creation)
		Enable("apply", false);*/
	TempMaterial = *mat;
	FillTextureList();
}


void ModelMaterialDialog::FillTextureList()
{
	Reset("mat_textures");
	for (int i=0;i<TempMaterial.NumTextures;i++){
		string img = ed->get_tex_image(NixLoadTexture(TempMaterial.TextureFile[i]));
		AddString("mat_textures", i2s(i) + "\\" + img + "\\" + file_secure(TempMaterial.TextureFile[i]));
	}
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
	SetInt("transparency_mode", 0);
	if (IsChecked("default_transparency")){
		Enable("transparency_mode", false);
		Enable("alpha_factor", false);
		Enable("alpha_source", false);
		Enable("alpha_dest", false);
		Enable("alpha_z_buffer", false);
	}else{
		Enable("transparency_mode", true);
		int s = GetInt("transparency_mode");
		Enable("alpha_factor", s == 3);
		Enable("alpha_source", s == 4);
		Enable("alpha_dest", s == 4);
		Enable("alpha_z_buffer", true);
	}
}

// textures
void ModelMaterialDialog::OnMatAddTextureLevel()
{
	if (TempMaterial.NumTextures >= MODEL_MAX_TEXTURES)
		ed->ErrorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MODEL_MAX_TEXTURES));
	else
		TempMaterial.TextureFile[TempMaterial.NumTextures ++] = "";
	FillTextureList();
}


void ModelMaterialDialog::OnMatTextures()
{
	int sel = GetInt("");
	if ((sel >= 0) && (sel < TempMaterial.NumTextures))
		if (ed->FileDialog(FDTexture, false, true)){
			TempMaterial.TextureFile[sel] = ed->DialogFile;
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			FillTextureList();
		}
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
		FillTextureList();
	}
}

void ModelMaterialDialog::OnMatEmptyTextureLevel()
{
	int sel = GetInt("mat_textures");
	if (sel >= 0){
		TempMaterial.TextureFile[sel] = "";
		FillTextureList();
	}
}

// material
void ModelMaterialDialog::OnDefaultMaterial()
{
	if (IsChecked("")){
		SetString("material_file","");
		Enable("material_file",false);
	}else
		Enable("material_file",true);
}

		/*case HMM_MATERIAL_FILE:
			dlg->Enable(HMM_MATERIAL_FILE,strlen(MaterialFile)>0);
			break;*/

void ModelMaterialDialog::OnFindMaterial()
{
	if (ed->FileDialog(FDMaterial, false, true)){
		SetString("material_file", ed->DialogFileNoEnding);
		Enable("material_file", true);
		Check("default_material",false);
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
	bool b = !IsChecked("");
	Enable("mat_am", b);
	Enable("mat_di", b);
	Enable("mat_sp", b);
	Enable("mat_em", b);
	Enable("mat_shininess", b);
}


void ModelMaterialDialog::ApplyData()
{
// material
	if (IsChecked("default_material"))
		mat->MaterialFile = "";
	else
		mat->MaterialFile = GetString("material_file");
	mat->Color[0] = GetColor("mat_am");
	mat->Color[1] = GetColor("mat_di");
	mat->Color[2] = GetColor("mat_sp");
	mat->Color[3] = GetColor("mat_em");
	mat->Shininess = GetInt("mat_shininess");
	mat->UserColor = !IsChecked("default_colors");
	mat->material = MetaLoadMaterial(mat->MaterialFile);
// transparency
	int s = GetInt("transparency_mode");
	if (s==1)
		mat->TransparencyMode=TransparencyModeColorKeySmooth;
	else if (s==2)
		mat->TransparencyMode=TransparencyModeColorKeyHard;
	else if (s==3)
		mat->TransparencyMode=TransparencyModeFactor;
	else if (s==4)
		mat->TransparencyMode=TransparencyModeFunctions;
	else
		mat->TransparencyMode=TransparencyModeNone;
	if (IsChecked("default_transparency"))
		mat->TransparencyMode=TransparencyModeDefault;
	mat->AlphaFactor = GetInt("alpha_factor");
	mat->AlphaSource = GetInt("alpha_source");
	mat->AlphaDestination = GetInt("alpha_dest");
	mat->AlphaZBuffer = IsChecked("alpha_z_buffer");
// textures
	mat->NumTextures = TempMaterial.NumTextures;
	for (int i=0;i<TempMaterial.NumTextures;i++){
		mat->TextureFile[i] = TempMaterial.TextureFile[i];
		mat->Texture[i] = NixLoadTexture(mat->TextureFile[i]);
	}
			/*if (m->NumTextures < skin->Sub[mmodel->CurrentMaterial].NumTextures){
				for (int i=0;i<4;i++)
					mmodel->Skin[i].Sub[mmodel->CurrentMaterial].NumTextures = m->NumTextures;
			}else if (m->NumTextures > skin->Sub[mmodel->CurrentMaterial].NumTextures){
				for (int i=0;i<4;i++)
					mmodel->Skin[i].Sub[mmodel->CurrentMaterial].NumTextures = m->NumTextures;
				// TODO ...new SkinVertices...
			}*/

	//StartChanging();
	mat->CheckTextures();
	//Change();
	//EndChanging();
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
