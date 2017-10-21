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

TerrainPropertiesDialog::TerrainPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data, int _index) :
	hui::Dialog("terrain_dialog", 400, 300, _parent, _allow_parent),
	Observer("TerrainPropertiesDialog")
{
	fromResource("terrain_dialog");
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->Terrains.num);

	event("cancel", std::bind(&TerrainPropertiesDialog::OnClose, this));
	event("hui:close", std::bind(&TerrainPropertiesDialog::OnClose, this));
	event("apply", std::bind(&TerrainPropertiesDialog::ApplyData, this));
	event("ok", std::bind(&TerrainPropertiesDialog::OnOk, this));

	event("add_texture_level", std::bind(&TerrainPropertiesDialog::OnAddTextureLevel, this));
	event("delete_texture_level", std::bind(&TerrainPropertiesDialog::OnDeleteTextureLevel, this));
	event("clear_texture_level", std::bind(&TerrainPropertiesDialog::OnClearTextureLevel, this));
	event("texture_map_complete", std::bind(&TerrainPropertiesDialog::OnTextureMapComplete, this));
	event("textures", std::bind(&TerrainPropertiesDialog::OnTextures, this));
	eventX("textures", "hui:change", std::bind(&TerrainPropertiesDialog::OnTexturesEdit, this));
	eventX("textures", "hui:select", std::bind(&TerrainPropertiesDialog::OnTexturesSelect, this));
	event("material_find", std::bind(&TerrainPropertiesDialog::OnMaterialFind, this));
	event("default_material", std::bind(&TerrainPropertiesDialog::OnDefaultMaterial, this));
	event("terrain_save_as", std::bind(&TerrainPropertiesDialog::OnSaveAs, this));

	subscribe(data);

	onUpdate(data, "");
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
	if (ed->fileDialog(FD_TEXTURE, false, true)){
		temp.TextureFile[n] = ed->dialog_file;
		FillTextureList();
	}
}


void TerrainPropertiesDialog::FillTextureList()
{
	Material *m = LoadMaterial(getString("material"));

	reset("textures");
	for (int i=0;i<temp.NumTextures;i++){
		nix::Texture *tex = nix::LoadTexture(temp.TextureFile[i]);
		if (tex)
			if (i < m->textures.num)
				tex = m->textures[i];
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
	int col = hui::GetEvent()->column;
	int row = hui::GetEvent()->row;
	if (col == 1)
		temp.TextureScale[row].x = s2f(getCell("textures", row, col)) / (float)temp.NumX;
	else if (col == 2)
		temp.TextureScale[row].z = s2f(getCell("textures", row, col)) / (float)temp.NumZ;
}



void TerrainPropertiesDialog::OnSaveAs()
{
	if (!ed->fileDialog(FD_TERRAIN, true, true))
		return;
	data->Terrains[index].Save(ed->dialog_file_complete);
	setString("filename", ed->dialog_file_no_ending);
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
	if (ed->fileDialog(FD_MATERIAL, false, true))
		setString("material", ed->dialog_file_no_ending);
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
	destroy();
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

void TerrainPropertiesDialog::onUpdate(Observable *o, const string &message)
{
	if (index >= data->Terrains.num){
		destroy();
		return;
	}

	Terrain *t = data->Terrains[index].terrain;
	assert(t);
	temp.FileName = data->Terrains[index].FileName;
	temp.NumX = t->num_x;
	temp.NumZ = t->num_z;
	temp.Pattern = t->pattern;
	temp.NumTextures = t->material->textures.num;
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
	destroy();
}


