/*
 * TerrainCreationDialog.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "TerrainCreationDialog.h"
#include "../../../Edward.h"

TerrainCreationDialog::TerrainCreationDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data) :
	hui::Dialog("new_terrain_dialog", 400, 300, _parent, _allow_parent),
	Observer("TerrainCreationDialog")
{
	fromResource("new_terrain_dialog");
	data = _data;

	event("cancel", std::bind(&TerrainCreationDialog::OnClose, this));
	event("hui:close", std::bind(&TerrainCreationDialog::OnClose, this));
	event("apply", std::bind(&TerrainCreationDialog::ApplyData, this));
	event("ok", std::bind(&TerrainCreationDialog::OnOk, this));

	event("height_image_find", std::bind(&TerrainCreationDialog::OnFindHeightmap, this));
	event("num_x", std::bind(&TerrainCreationDialog::OnSizeChange, this));
	event("num_y", std::bind(&TerrainCreationDialog::OnSizeChange, this));
	event("terrain_x", std::bind(&TerrainCreationDialog::OnSizeChange, this));
	event("terrain_y", std::bind(&TerrainCreationDialog::OnSizeChange, this));

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
	if (ed->fileDialog(FD_TEXTURE, false, false))
		setString("height_image", ed->dialog_file_complete);
}



void TerrainCreationDialog::OnSizeChange()
{
	setFloat("pattern_x", getFloat("terrain_x") / (float)getInt("num_x"));
	setFloat("pattern_y", getFloat("terrain_y") / (float)getInt("num_y"));
}


