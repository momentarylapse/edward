/*
 * TerrainCreationDialog.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "TerrainCreationDialog.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"

TerrainCreationDialog::TerrainCreationDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data) :
	hui::Dialog("new_terrain_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("new_terrain_dialog");
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

	LoadData();
}

TerrainCreationDialog::~TerrainCreationDialog()
{
}

void TerrainCreationDialog::ApplyData()
{
}



void TerrainCreationDialog::LoadData()
{
	set_float("height_factor", 100);
	set_int("num_x", 64);
	set_int("num_y", 64);
	set_float("terrain_x", 1000);
	set_float("terrain_y", 1000);
	OnSizeChange();
}





void TerrainCreationDialog::OnOk()
{
	destroy();
}



void TerrainCreationDialog::OnClose()
{
	destroy();
}



void TerrainCreationDialog::OnFindHeightmap()
{
	if (storage->file_dialog(FD_TEXTURE, false, false))
		set_string("height_image", storage->dialog_file_complete);
}



void TerrainCreationDialog::OnSizeChange()
{
	set_float("pattern_x", get_float("terrain_x") / (float)get_int("num_x"));
	set_float("pattern_y", get_float("terrain_y") / (float)get_int("num_y"));
}


