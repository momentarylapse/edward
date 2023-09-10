/*
 * TerrainCreationDialog.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "TerrainCreationDialog.h"
#include "../../../EdwardWindow.h"
#include "../../../storage/Storage.h"

TerrainCreationDialog::TerrainCreationDialog(EdwardWindow *_ed, bool _allow_parent, DataWorld *_data) :
	hui::Dialog("new_terrain_dialog", 400, 300, _ed, _allow_parent)
{
	from_resource("new_terrain_dialog");
	ed = _ed;
	data = _data;

	event("cancel", [=]{ OnClose(); });
	event("hui:close", [=]{ OnClose(); });
	event("apply", [=]{ ApplyData(); });
	event("ok", [=]{ OnOk(); });

	event("height_image_find", [=]{ OnFindHeightmap(); });
	event("num_x", [=]{ OnSizeChange(); });
	event("num_y", [=]{ OnSizeChange(); });
	event("terrain_x", [=]{ OnSizeChange(); });
	event("terrain_y", [=]{ OnSizeChange(); });

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
	request_destroy();
}



void TerrainCreationDialog::OnClose()
{
	request_destroy();
}



void TerrainCreationDialog::OnFindHeightmap() {
	ed->storage->file_dialog(FD_TEXTURE, false, false).on([this] (const auto& p) {
		set_string("height_image", p.complete.str());
	});
}



void TerrainCreationDialog::OnSizeChange() {
	set_float("pattern_x", get_float("terrain_x") / (float)get_int("num_x"));
	set_float("pattern_y", get_float("terrain_y") / (float)get_int("num_y"));
}


