/*
 * TerrainHeightmapDialog.cpp
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#include "TerrainHeightmapDialog.h"
#include "../../../Action/World/Terrain/ActionWorldTerrainApplyHeightmap.h"
#include "../../../Edward.h"

TerrainHeightmapDialog::TerrainHeightmapDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data) :
	hui::Dialog("terrain_heightmap_dialog", 400, 300, _parent, _allow_parent),
	Observer("TerrainHeightmapDialog")
{
	from_resource("terrain_heightmap_dialog");
	data = _data;

	event("cancel", std::bind(&TerrainHeightmapDialog::OnClose, this));
	event("hui:close", std::bind(&TerrainHeightmapDialog::OnClose, this));
	event("apply", std::bind(&TerrainHeightmapDialog::ApplyData, this));
	event("ok", std::bind(&TerrainHeightmapDialog::OnOk, this));

	event("height_image_find", std::bind(&TerrainHeightmapDialog::OnFindHeightmap, this));
	event("stretch_x", std::bind(&TerrainHeightmapDialog::OnSizeChange, this));
	event("stretch_z", std::bind(&TerrainHeightmapDialog::OnSizeChange, this));
	event("filter_image_find", std::bind(&TerrainHeightmapDialog::OnFindFilter, this));
	event_xp("preview", "hui:draw", std::bind(&TerrainHeightmapDialog::OnPreviewDraw, this, std::placeholders::_1));

	enable("ok", false);

	subscribe(data);

	stretch_x = 1;
	stretch_z = 1;
	LoadData();
}

TerrainHeightmapDialog::~TerrainHeightmapDialog()
{
	unsubscribe(data);
}

void TerrainHeightmapDialog::ApplyData()
{
}



void TerrainHeightmapDialog::OnSizeChange()
{
	stretch_x = get_float("stretch_x");
	stretch_z = get_float("stretch_z");
	redraw("preview");
}



void TerrainHeightmapDialog::OnFindFilter()
{
	if (ed->file_dialog(FD_TEXTURE, false, false)){
		filter_file = ed->dialog_file_complete;
		set_string("filter_image", ed->dialog_file);
		filter.load(filter_file);
		redraw("preview");
	}
}



void TerrainHeightmapDialog::on_update(Observable *o, const string &message)
{
}



void TerrainHeightmapDialog::OnFindHeightmap()
{
	if (ed->file_dialog(FD_TEXTURE, false, false)){
		heightmap_file = ed->dialog_file_complete;
		set_string("height_image", ed->dialog_file);
		heightmap.load(heightmap_file);
		redraw("preview");
		enable("ok", true);
	}
}


static float c2f(const color &c)
{
	return (c.r + c.g + c.b) / 3.0f;
}

// texture interpolation (without repeating the last half pixel)
static float im_interpolate(const Image &im, float x, float y, float stretch_x, float stretch_y)
{
	stretch_x *= im.width;
	stretch_y *= im.height;
	x = clampf(x * stretch_x, 0.5f, stretch_x - 0.5f);
	y = clampf(y * stretch_y, 0.5f, stretch_y - 0.5f);
	return c2f(im.get_pixel_interpolated(x, y));
}

void TerrainHeightmapDialog::OnPreviewDraw(Painter *c)
{
	if (heightmap.is_empty()){
		c->set_color(Black);
		c->draw_rect(0, 0, c->width, c->height);
	}else{
		Image m;
		int w = c->width, h = c->height;
		m.create(w, h, White);
		for (int x=0;x<w;x++)
			for (int y=0;y<h;y++){
				float hmx = (float)x / (float)w;
				float hmy = (float)y / (float)h;
				float f = im_interpolate(heightmap, hmx, hmy, stretch_x, stretch_z);
				if (!filter.is_empty())
					f *= im_interpolate(filter, hmx, hmy, 1, 1);
				m.set_pixel(x, y, color(1, f, f, f));
			}
		c->draw_image(0, 0, &m);
	}
}



void TerrainHeightmapDialog::OnOk()
{
	float height_factor = get_float("height_factor");
	bool additive = is_checked("height_op:add");
	data->execute(new ActionWorldTerrainApplyHeightmap(data, heightmap_file, height_factor, stretch_x, stretch_z, filter_file));//, additive));
	destroy();
}



void TerrainHeightmapDialog::OnClose()
{
	destroy();
}



void TerrainHeightmapDialog::LoadData()
{
	set_float("stretch_x", stretch_x);
	set_float("stretch_z", stretch_z);
	set_float("height_factor", 100);
	check("height_op:set", true);
}


