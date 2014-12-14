/*
 * TerrainPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "TerrainPropertiesDialog.h"
#include "../../../Action/World/Terrain/ActionWorldEditTerrain.h"
#include "../../../Edward.h"
#include "../../../x/terrain.h"
#include "../../../x/material.h"
#include "../../../lib/nix/nix.h"
#include <assert.h>

string file_secure(const string &filename); // -> ModelPropertiesDialog

TerrainPropertiesDialog::TerrainPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data, int _index) :
	HuiWindow("terrain_dialog", _parent, _allow_parent)
{
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->Terrains.num);

	event("cancel", this, &TerrainPropertiesDialog::OnClose);
	event("hui:close", this, &TerrainPropertiesDialog::OnClose);
	event("apply", this, &TerrainPropertiesDialog::ApplyData);
	event("ok", this, &TerrainPropertiesDialog::OnOk);

	event("add_texture_level", this, &TerrainPropertiesDialog::OnAddTextureLevel);
	event("delete_texture_level", this, &TerrainPropertiesDialog::OnDeleteTextureLevel);
	event("clear_texture_level", this, &TerrainPropertiesDialog::OnClearTextureLevel);
	event("texture_map_complete", this, &TerrainPropertiesDialog::OnTextureMapComplete);
	event("textures", this, &TerrainPropertiesDialog::OnTextures);
	eventX("textures", "hui:change", this, &TerrainPropertiesDialog::OnTexturesEdit);
	eventX("textures", "hui:select", this, &TerrainPropertiesDialog::OnTexturesSelect);
	event("material_find", this, &TerrainPropertiesDialog::OnMaterialFind);
	event("default_material", this, &TerrainPropertiesDialog::OnDefaultMaterial);
	event("terrain_save_as", this, &TerrainPropertiesDialog::OnSaveAs);

	subscribe(data);

	onUpdate(data);
}

TerrainPropertiesDialog::~TerrainPropertiesDialog()
{
	unsubscribe(data);
}

void TerrainPropertiesDialog::ApplyData()
{
	temp.MaterialFile = getString("material");
	data->execute(new ActionWorldEditTerrain(index, temp));
}



void TerrainPropertiesDialog::OnTextures()
{
	int n = getInt("textures");
	if (ed->fileDialog(FDTexture, false, true)){
		temp.TextureFile[n] = ed->DialogFile;
		FillTextureList();
	}
}


void TerrainPropertiesDialog::FillTextureList()
{
	Material *m = LoadMaterial(getString("material"));

	reset("textures");
	for (int i=0;i<temp.NumTextures;i++){
		NixTexture *tex = NixLoadTexture(temp.TextureFile[i]);
		if (tex)
			if (i < m->num_textures)
				tex = m->texture[i];
		string img = ed->get_tex_image(tex);
		addString("textures", format("%d\\%.5f\\%.5f\\%s\\%s", i, temp.TextureScale[i].x * (float)temp.NumX, temp.TextureScale[i].z * (float)temp.NumZ, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	enable("delete_texture_level", false);
	enable("clear_texture_level", false);
	enable("texture_map_complete", false);
}



void TerrainPropertiesDialog::LoadData()
{
	setString("material", temp.MaterialFile);
	check("default_material", (temp.MaterialFile == ""));
	enable("material", false);//(temp.MaterialFile != ""));
	FillTextureList();

	setString("filename", temp.FileName);
	setInt("num_x", temp.NumX);
	setInt("num_z", temp.NumZ);
	setFloat("pattern_x", temp.Pattern.x);
	setFloat("pattern_z", temp.Pattern.z);
}



void TerrainPropertiesDialog::OnTexturesEdit()
{
	int col = HuiGetEvent()->column;
	int row = HuiGetEvent()->row;
	if (col == 1)
		temp.TextureScale[row].x = s2f(getCell("textures", row, col)) / (float)temp.NumX;
	else if (col == 2)
		temp.TextureScale[row].z = s2f(getCell("textures", row, col)) / (float)temp.NumZ;
}



void TerrainPropertiesDialog::OnSaveAs()
{
	if (!ed->fileDialog(FDTerrain, true, true))
		return;
	data->Terrains[index].Save(ed->DialogFileComplete);
	setString("filename", ed->DialogFileNoEnding);
}



void TerrainPropertiesDialog::OnDeleteTextureLevel()
{
	int n = getInt("textures");
	if (temp.NumTextures <= 1){
		ed->errorBox(_("Mindestens eine Textur-Ebene muss existieren!"));
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
	setString("material", "");
}



void TerrainPropertiesDialog::OnMaterialFind()
{
	if (ed->fileDialog(FDMaterial, false, true))
		setString("material", ed->DialogFileNoEnding);
}



void TerrainPropertiesDialog::OnTextureMapComplete()
{
	int s = getInt("textures");
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
		ed->errorBox(format(_("Es sind nur maximal %d Texturen pro Terrain erlaubt!"), MATERIAL_MAX_TEXTURES));
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
	int s = getInt("textures");
	enable("delete_texture_level", s >= 0);
	enable("clear_texture_level", s >= 0);
	enable("texture_map_complete", s >= 0);
}

void TerrainPropertiesDialog::OnClearTextureLevel()
{
	int s = getInt("textures");
	if (s >= 0)
		temp.TextureFile[s] = "";

	FillTextureList();
}

void TerrainPropertiesDialog::onUpdate(Observable *o)
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


