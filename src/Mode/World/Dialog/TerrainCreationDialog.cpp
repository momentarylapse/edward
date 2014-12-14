/*
 * TerrainCreationDialog.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "TerrainCreationDialog.h"
#include "../../../Edward.h"

TerrainCreationDialog::TerrainCreationDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	HuiWindow("new_terrain_dialog", _parent, _allow_parent),
	Observer("TerrainCreationDialog")
{
	data = _data;

	event("cancel", this, &TerrainCreationDialog::OnClose);
	event("hui:close", this, &TerrainCreationDialog::OnClose);
	event("apply", this, &TerrainCreationDialog::ApplyData);
	event("ok", this, &TerrainCreationDialog::OnOk);

	event("height_image_find", this, &TerrainCreationDialog::OnFindHeightmap);
	event("num_x", this, &TerrainCreationDialog::OnSizeChange);
	event("num_y", this, &TerrainCreationDialog::OnSizeChange);
	event("terrain_x", this, &TerrainCreationDialog::OnSizeChange);
	event("terrain_y", this, &TerrainCreationDialog::OnSizeChange);

	subscribe(data);

	LoadData();
}

TerrainCreationDialog::~TerrainCreationDialog()
{
	unsubscribe(data);
}

void TerrainCreationDialog::ApplyData()
{
}



void TerrainCreationDialog::LoadData()
{
	setFloat("height_factor", 100);
	setInt("num_x", 64);
	setInt("num_y", 64);
	setFloat("terrain_x", 1000);
	setFloat("terrain_y", 1000);
	OnSizeChange();
}



void TerrainCreationDialog::onUpdate(Observable *o, const string &message)
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
	if (ed->fileDialog(FDTexture, false, false))
		setString("height_image", ed->DialogFileComplete);
}



void TerrainCreationDialog::OnSizeChange()
{
	setFloat("pattern_x", getFloat("terrain_x") / (float)getInt("num_x"));
	setFloat("pattern_y", getFloat("terrain_y") / (float)getInt("num_y"));
}


