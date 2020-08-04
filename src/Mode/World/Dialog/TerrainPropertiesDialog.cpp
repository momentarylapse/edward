/*
 * TerrainPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "TerrainPropertiesDialog.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Data/World/WorldTerrain.h"
#include "../../../Action/World/Terrain/ActionWorldEditTerrain.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
#include "../../../x/terrain.h"
#include "../../../x/material.h"
#include "../../../lib/nix/nix.h"
#include <assert.h>

string file_secure(const Path &filename); // -> ModelPropertiesDialog

TerrainPropertiesDialog::TerrainPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data, int _index) :
	hui::Dialog("terrain_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("terrain_dialog");
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->terrains.num);

	event("cancel", [=]{ OnClose(); });
	event("hui:close", [=]{ OnClose(); });
	event("apply", [=]{ ApplyData(); });
	event("ok", [=]{ OnOk(); });

	event("add_texture_level", [=]{ OnAddTextureLevel(); });
	event("delete_texture_level", [=]{ OnDeleteTextureLevel(); });
	event("clear_texture_level", [=]{ OnClearTextureLevel(); });
	event("texture_map_complete", [=]{ OnTextureMapComplete(); });
	event("textures", [=]{ OnTextures(); });
	event_x("textures", "hui:change", [=]{ OnTexturesEdit(); });
	event_x("textures", "hui:select", [=]{ OnTexturesSelect(); });
	event("material_find", [=]{ OnMaterialFind(); });
	event("default_material", [=]{ OnDefaultMaterial(); });
	event("terrain_save_as", [=]{ OnSaveAs(); });

	data->subscribe(this, [=]{ update_data(); });

	update_data();
}

TerrainPropertiesDialog::~TerrainPropertiesDialog()
{
	data->unsubscribe(this);
}

void TerrainPropertiesDialog::ApplyData()
{
	temp.MaterialFile = get_string("material");
	data->execute(new ActionWorldEditTerrain(index, temp));
}



void TerrainPropertiesDialog::OnTextures()
{
	int n = get_int("textures");
	if (storage->file_dialog(FD_TEXTURE, false, true)){
		temp.TextureFile[n] = storage->dialog_file;
		FillTextureList();
	}
}


void TerrainPropertiesDialog::FillTextureList()
{
	Material *m = LoadMaterial(get_string("material"));

	reset("textures");
	for (int i=0;i<temp.NumTextures;i++){
		nix::Texture *tex = nix::LoadTexture(temp.TextureFile[i]);
		if (tex)
			if (i < m->textures.num)
				tex = m->textures[i];
		string img = ed->get_tex_image(tex);
		add_string("textures", format("%d\\%.5f\\%.5f\\%s\\%s", i, temp.TextureScale[i].x * (float)temp.NumX, temp.TextureScale[i].z * (float)temp.NumZ, img, file_secure(temp.TextureFile[i])));
	}
	enable("delete_texture_level", false);
	enable("clear_texture_level", false);
	enable("texture_map_complete", false);
}



void TerrainPropertiesDialog::LoadData()
{
	set_string("material", temp.MaterialFile.str());
	check("default_material", temp.MaterialFile.is_empty());
	enable("material", false);//(temp.MaterialFile != ""));
	FillTextureList();

	set_string("filename", temp.FileName.str());
	set_int("num_x", temp.NumX);
	set_int("num_z", temp.NumZ);
	set_float("pattern_x", temp.Pattern.x);
	set_float("pattern_z", temp.Pattern.z);
}



void TerrainPropertiesDialog::OnTexturesEdit()
{
	int col = hui::GetEvent()->column;
	int row = hui::GetEvent()->row;
	if (col == 1)
		temp.TextureScale[row].x = s2f(get_cell("textures", row, col)) / (float)temp.NumX;
	else if (col == 2)
		temp.TextureScale[row].z = s2f(get_cell("textures", row, col)) / (float)temp.NumZ;
}



void TerrainPropertiesDialog::OnSaveAs()
{
	if (!storage->file_dialog(FD_TERRAIN, true, true))
		return;
	data->terrains[index].save(storage->dialog_file_complete);
	set_string("filename", storage->dialog_file_no_ending.str());
}



void TerrainPropertiesDialog::OnDeleteTextureLevel()
{
	int n = get_int("textures");
	if (temp.NumTextures <= 1){
		ed->error_box(_("There has to be at least one texture level!"));
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
	set_string("material", "");
}



void TerrainPropertiesDialog::OnMaterialFind()
{
	if (storage->file_dialog(FD_MATERIAL, false, true))
		set_string("material", storage->dialog_file_no_ending.str());
}



void TerrainPropertiesDialog::OnTextureMapComplete()
{
	int s = get_int("textures");
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
		ed->error_box(format(_("No more than %d textures per terrain allowed!"), MATERIAL_MAX_TEXTURES));
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
	int s = get_int("textures");
	enable("delete_texture_level", s >= 0);
	enable("clear_texture_level", s >= 0);
	enable("texture_map_complete", s >= 0);
}

void TerrainPropertiesDialog::OnClearTextureLevel()
{
	int s = get_int("textures");
	if (s >= 0)
		temp.TextureFile[s] = "";

	FillTextureList();
}

void TerrainPropertiesDialog::update_data() {
	if (index >= data->terrains.num) {
		destroy();
		return;
	}

	Terrain *t = data->terrains[index].terrain;
	assert(t);
	temp.FileName = data->terrains[index].filename;
	temp.NumX = t->num_x;
	temp.NumZ = t->num_z;
	temp.Pattern = t->pattern;
	temp.NumTextures = t->material->textures.num;
	for (int i=0; i<temp.NumTextures; i++) {
		temp.TextureFile[i] = t->texture_file[i];
		temp.TextureScale[i] = t->texture_scale[i];
	}
	temp.MaterialFile = t->material_file;
	LoadData();
}



void TerrainPropertiesDialog::OnOk() {
	ApplyData();
	destroy();
}


