/*
 * TerrainCreationDialog.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "TerrainCreationDialog.h"
#include "../../../Edward.h"

TerrainCreationDialog::TerrainCreationDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("new_terrain_dialog");

	EventM("cancel", this, &TerrainCreationDialog::OnClose);
	EventM("hui:close", this, &TerrainCreationDialog::OnClose);
	EventM("apply", this, &TerrainCreationDialog::ApplyData);
	EventM("ok", this, &TerrainCreationDialog::OnOk);

	EventM("height_image_find", this, &TerrainCreationDialog::OnFindHeightmap);
	EventM("num_x", this, &TerrainCreationDialog::OnSizeChange);
	EventM("num_y", this, &TerrainCreationDialog::OnSizeChange);
	EventM("terrain_x", this, &TerrainCreationDialog::OnSizeChange);
	EventM("terrain_y", this, &TerrainCreationDialog::OnSizeChange);

	Subscribe(data);

	LoadData();
}

TerrainCreationDialog::~TerrainCreationDialog()
{
	Unsubscribe(data);
}

void TerrainCreationDialog::ApplyData()
{
}



void TerrainCreationDialog::LoadData()
{
	SetFloat("height_factor", 100);
	SetInt("num_x", 64);
	SetInt("num_y", 64);
	SetFloat("terrain_x", 1000);
	SetFloat("terrain_y", 1000);
	OnSizeChange();
}



void TerrainCreationDialog::OnUpdate(Observable *o)
{
}



void TerrainCreationDialog::OnOk()
{
	delete(this);
}



void TerrainCreationDialog::OnClose()
{
	delete(this);
}



void TerrainCreationDialog::OnFindHeightmap()
{
	if (ed->FileDialog(FDTexture, false, false))
		SetString("height_image", ed->DialogFileComplete);
}



void TerrainCreationDialog::OnSizeChange()
{
	SetFloat("pattern_x", GetFloat("terrain_x") / (float)GetInt("num_x"));
	SetFloat("pattern_y", GetFloat("terrain_y") / (float)GetInt("num_y"));
}


