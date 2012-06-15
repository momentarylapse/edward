/*
 * TerrainPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "TerrainPropertiesDialog.h"
#include "../../../Action/World/ActionWorldEditTerrain.h"
#include "../../../Edward.h"
#include <assert.h>

string file_secure(const string &filename); // -> ModelPropertiesDialog

TerrainPropertiesDialog::TerrainPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data, int _index) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->Terrain.num);

	// dialog
	FromResource("terrain_dialog");

	EventM("cancel", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnClose);
	EventM("apply", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::ApplyData);
	EventM("ok", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnOk);

	EventM("add_texture_level", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnAddTextureLevel);
	EventM("delete_texture_level", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnDeleteTextureLevel);
	EventM("clear_texture_level", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnClearTextureLevel);
	EventM("texture_map_complete", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnTextureMapComplete);
	EventM("textures", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnTextures);
	EventMX("textures", "hui:change", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnTexturesEdit);
	EventMX("textures", "hui:select", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnTexturesSelect);
	EventM("material_find", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnMaterialFind);
	EventM("default_material", this, (void(HuiEventHandler::*)())&TerrainPropertiesDialog::OnDefaultMaterial);

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
	Material *m = MetaLoadMaterial(GetString("material"));

	Reset("textures");
	for (int i=0;i<temp.NumTextures;i++){
		int tex = NixLoadTexture(temp.TextureFile[i]);
		if (tex < 0)
			if (i < m->num_textures)
				tex = m->texture[i];
		string img = ed->get_tex_image(tex);
		AddString("textures", format("%d\\%.5f\\%.5f\\%s\\%s", i, temp.TextureScale[i].x, temp.TextureScale[i].z, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	Enable("delete_texture_level", false);
	Enable("clear_texture_level", false);
	Enable("texture_map_complete", false);
}



void TerrainPropertiesDialog::LoadData()
{
	FillTextureList();
	SetString("material", temp.MaterialFile);
	Check("default_material", (temp.MaterialFile == ""));
	Enable("material", false);//(temp.MaterialFile != ""));

	SetString("", temp.FileName);
	SetFloat("num_x", temp.NumX);
	SetFloat("num_z", temp.NumZ);
	SetFloat("pattern_x", temp.Pattern.x);
	SetFloat("pattern_z", temp.Pattern.z);
}



void TerrainPropertiesDialog::OnTexturesEdit()
{
	int col = HuiGetEvent()->column;
	int row = HuiGetEvent()->row;
	if (col == 1)
		temp.TextureScale[row].x = s2f(GetCell("textures", row, col));
	else if (col == 2)
		temp.TextureScale[row].y = s2f(GetCell("textures", row, col));
}



void TerrainPropertiesDialog::OnSaveAs()
{
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
	if (temp.NumTextures >= WORLD_MAX_TEXTURES_PER_TERRAIN){
		ed->ErrorBox(format(_("Es sind nur maximal %d Texturen pro Terrain erlaubt!"), WORLD_MAX_TEXTURES_PER_TERRAIN));
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
	if (index >= data->Terrain.num){
		delete(this);
		return;
	}

	CTerrain *t = data->Terrain[index].terrain;
	assert(t);
	temp.FileName = data->Terrain[index].FileName;
	temp.NumX = t->num_x;
	temp.NumZ = t->num_z;
	temp.Pattern = t->pattern;
	temp.NumTextures = t->num_textures;
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


