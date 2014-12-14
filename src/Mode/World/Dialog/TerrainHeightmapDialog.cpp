/*
 * TerrainHeightmapDialog.cpp
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#include "TerrainHeightmapDialog.h"
#include "../../../Action/World/Terrain/ActionWorldTerrainApplyHeightmap.h"
#include "../../../Edward.h"

TerrainHeightmapDialog::TerrainHeightmapDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	HuiWindow("terrain_heightmap_dialog", _parent, _allow_parent)
{
	data = _data;

	event("cancel", this, &TerrainHeightmapDialog::OnClose);
	event("hui:close", this, &TerrainHeightmapDialog::OnClose);
	event("apply", this, &TerrainHeightmapDialog::ApplyData);
	event("ok", this, &TerrainHeightmapDialog::OnOk);

	event("height_image_find", this, &TerrainHeightmapDialog::OnFindHeightmap);
	event("stretch_x", this, &TerrainHeightmapDialog::OnSizeChange);
	event("stretch_z", this, &TerrainHeightmapDialog::OnSizeChange);
	event("filter_image_find", this, &TerrainHeightmapDialog::OnFindFilter);
	eventX("preview", "hui:draw", this, &TerrainHeightmapDialog::OnPreviewDraw);

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
	stretch_x = getFloat("stretch_x");
	stretch_z = getFloat("stretch_z");
	redraw("preview");
}



void TerrainHeightmapDialog::OnFindFilter()
{
	if (ed->fileDialog(FDTexture, false, false)){
		filter_file = ed->DialogFileComplete;
		setString("filter_image", ed->DialogFile);
		filter.load(filter_file);
		redraw("preview");
	}
}



void TerrainHeightmapDialog::onUpdate(Observable *o)
{
}



void TerrainHeightmapDialog::OnFindHeightmap()
{
	if (ed->fileDialog(FDTexture, false, false)){
		heightmap_file = ed->DialogFileComplete;
		setString("height_image", ed->DialogFile);
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
	return c2f(im.getPixelInterpolated(x, y));
}

void TerrainHeightmapDialog::OnPreviewDraw()
{
	HuiPainter *c = beginDraw("preview");
	if (heightmap.isEmpty()){
		c->setColor(Black);
		c->drawRect(0, 0, c->width, c->height);
	}else{
		Image m;
		int w = c->width, h = c->height;
		m.create(w, h, White);
		for (int x=0;x<w;x++)
			for (int y=0;y<h;y++){
				float hmx = (float)x / (float)w;
				float hmy = (float)y / (float)h;
				float f = im_interpolate(heightmap, hmx, hmy, stretch_x, stretch_z);
				if (!filter.isEmpty())
					f *= im_interpolate(filter, hmx, hmy, 1, 1);
				m.setPixel(x, y, color(1, f, f, f));
			}
		c->drawImage(0, 0, m);
	}
	c->end();
}



void TerrainHeightmapDialog::OnOk()
{
	float height_factor = getFloat("height_factor");
	bool additive = isChecked("height_op:add");
	data->execute(new ActionWorldTerrainApplyHeightmap(data, heightmap_file, height_factor, stretch_x, stretch_z, filter_file));//, additive));
	delete(this);
}



void TerrainHeightmapDialog::OnClose()
{
	delete(this);
}



void TerrainHeightmapDialog::LoadData()
{
	setFloat("stretch_x", stretch_x);
	setFloat("stretch_z", stretch_z);
	setFloat("height_factor", 100);
	check("height_op:set", true);
}


