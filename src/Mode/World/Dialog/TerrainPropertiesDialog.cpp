/*
 * TerrainPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "TerrainPropertiesDialog.h"
#include "../../../Action/World/ActionWorldEditTerrain.h"
#include "../../../Edward.h"
#include "../../../x/terrain.h"
#include "../../../x/material.h"
#include <assert.h>

string file_secure(const string &filename); // -> ModelPropertiesDialog

TerrainPropertiesDialog::TerrainPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data, int _index) :
	HuiWindow("terrain_dialog", _parent, _allow_parent)
{
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->Terrains.num);

	EventM("cancel", this, &TerrainPropertiesDialog::OnClose);
	EventM("hui:close", this, &TerrainPropertiesDialog::OnClose);
	EventM("apply", this, &TerrainPropertiesDialog::ApplyData);
	EventM("ok", this, &TerrainPropertiesDialog::OnOk);

	EventM("add_texture_level", this, &TerrainPropertiesDialog::OnAddTextureLevel);
	EventM("delete_texture_level", this, &TerrainPropertiesDialog::OnDeleteTextureLevel);
	EventM("clear_texture_level", this, &TerrainPropertiesDialog::OnClearTextureLevel);
	EventM("texture_map_complete", this, &TerrainPropertiesDialog::OnTextureMapComplete);
	EventM("textures", this, &TerrainPropertiesDialog::OnTextures);
	EventMX("textures", "hui:change", this, &TerrainPropertiesDialog::OnTexturesEdit);
	EventMX("textures", "hui:select", this, &TerrainPropertiesDialog::OnTexturesSelect);
	EventM("material_find", this, &TerrainPropertiesDialog::OnMaterialFind);
	EventM("default_material", this, &TerrainPropertiesDialog::OnDefaultMaterial);
	EventM("terrain_save_as", this, &TerrainPropertiesDialog::OnSaveAs);

	Subscribe(data);

	OnUpdate(data);
}

TerrainPropertiesDialog::~TerrainPropertiesDialog()
{
	Unsubscribe(data);
}

void TerrainPropertiesDialog::ApplyData()
{
	temp.MaterialFile = GetString("material");
	data->Execute(new ActionWorldEditTerrain(index, temp));
}



void TerrainPropertiesDialog::OnTextures()
{
	int n = GetInt("textures");
	if (ed->FileDialog(FDTexture, false, true)){
		temp.TextureFile[n] = ed->DialogFile;
		FillTextureList();
	}
}


void TerrainPropertiesDialog::FillTextureList()
{
	Material *m = LoadMaterial(GetString("material"));

	Reset("textures");
	for (int i=0;i<temp.NumTextures;i++){
		int tex = NixLoadTexture(temp.TextureFile[i]);
		if (tex < 0)
			if (i < m->num_textures)
				tex = m->texture[i];
		string img = ed->get_tex_image(tex);
		AddString("textures", format("%d\\%.5f\\%.5f\\%s\\%s", i, temp.TextureScale[i].x * (float)temp.NumX, temp.TextureScale[i].z * (float)temp.NumZ, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	Enable("delete_texture_level", false);
	Enable("clear_texture_level", false);
	Enable("texture_map_complete", false);
}



void TerrainPropertiesDialog::LoadData()
{
	SetString("material", temp.MaterialFile);
	Check("default_material", (temp.MaterialFile == ""));
	Enable("material", false);//(temp.MaterialFile != ""));
	FillTextureList();

	SetString("filename", temp.FileName);
	SetInt("num_x", temp.NumX);
	SetInt("num_z", temp.NumZ);
	SetFloat("pattern_x", temp.Pattern.x);
	SetFloat("pattern_z", temp.Pattern.z);
}



void TerrainPropertiesDialog::OnTexturesEdit()
{
	int col = HuiGetEvent()->column;
	int row = HuiGetEvent()->row;
	if (col == 1)
		temp.TextureScale[row].x = s2f(GetCell("textures", row, col)) / (float)temp.NumX;
	else if (col == 2)
		temp.TextureScale[row].z = s2f(GetCell("textures", row, col)) / (float)temp.NumZ;
}



void TerrainPropertiesDialog::OnSaveAs()
{
	if (!ed->FileDialog(FDTerrain, true, true))
		return;
	data->Terrains[index].Save(ed->DialogFileComplete);
	SetString("filename", ed->DialogFileNoEnding);
}



void TerrainPropertiesDialog::OnDeleteTextureLevel()
{
	int n = GetInt("textures");
	if (temp.NumTextures <= 1){
		ed->ErrorBox(_("Mindestens eine Textur-Ebene muss existieren!"));
		return;
	}
	for (int i=n;i<temp.NumTextures-1;i++){
		temp.TextureFile[i] = temp.TextureFile[i+1];
		temp.TextureScale[i] = temp.TextureScale[i+1];
	}
	temp.NumTextures --;

	FillTextureList();
}



void TerrainPropertiesDialog::OnDefaultMaterial()
{
	SetString("material", "");
}



void TerrainPropertiesDialog::OnMaterialFind()
{
	if (ed->FileDialog(FDMaterial, false, true))
		SetString("material", ed->DialogFileNoEnding);
}



void TerrainPropertiesDialog::OnTextureMapComplete()
{
	int s = GetInt("textures");
	temp.TextureScale[s].x = 1.0f / temp.NumX;
	temp.TextureScale[s].z = 1.0f / temp.NumZ;

	FillTextureList();
}



void TerrainPropertiesDialog::OnClose()
{
	delete(this);
}



void TerrainPropertiesDialog::OnAddTextureLevel()
{
	if (temp.NumTextures >= MATERIAL_MAX_TEXTURES){
		ed->ErrorBox(format(_("Es sind nur maximal %d Texturen pro Terrain erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	int l = temp.NumTextures;
	temp.TextureFile[l] = "";
	temp.TextureScale[l] = vector(0.1f,0,0.1f);
	temp.NumTextures ++;

	FillTextureList();
}

void TerrainPropertiesDialog::OnTexturesSelect()
{
	int s = GetInt("textures");
	Enable("delete_texture_level", s >= 0);
	Enable("clear_texture_level", s >= 0);
	Enable("texture_map_complete", s >= 0);
}

void TerrainPropertiesDialog::OnClearTextureLevel()
{
	int s = GetInt("textures");
	if (s >= 0)
		temp.TextureFile[s] = "";

	FillTextureList();
}

void TerrainPropertiesDialog::OnUpdate(Observable *o)
{
	if (index >= data->Terrains.num){
		delete(this);
		return;
	}

	Terrain *t = data->Terrains[index].terrain;
	assert(t);
	temp.FileName = data->Terrains[index].FileName;
	temp.NumX = t->num_x;
	temp.NumZ = t->num_z;
	temp.Pattern = t->pattern;
	temp.NumTextures = t->material->num_textures;
	for (int i=0;i<temp.NumTextures;i++){
		temp.TextureFile[i] = t->texture_file[i];
		temp.TextureScale[i] = t->texture_scale[i];
	}
	temp.MaterialFile = t->material_file;
	LoadData();
}



void TerrainPropertiesDialog::OnOk()
{
	ApplyData();
	delete(this);
}


