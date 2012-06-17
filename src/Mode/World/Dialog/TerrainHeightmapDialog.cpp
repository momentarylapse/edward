/*
 * TerrainHeightmapDialog.cpp
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#include "TerrainHeightmapDialog.h"
#include "../../../Action/World/ActionWorldTerrainApplyHeightmap.h"
#include "../../../Edward.h"

TerrainHeightmapDialog::TerrainHeightmapDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("terrain_heightmap_dialog");

	EventM("cancel", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnClose);
	EventM("apply", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::ApplyData);
	EventM("ok", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnOk);

	EventM("height_image_find", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnFindHeightmap);
	EventM("stretch_x", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnSizeChange);
	EventM("stretch_z", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnSizeChange);
	EventM("filter_image_find", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnFindFilter);
	EventMX("preview", "hui:redraw", this, (void(HuiEventHandler::*)())&TerrainHeightmapDialog::OnPreviewDraw);

	Enable("ok", false);

	Subscribe(data);

	stretch_x = 1;
	stretch_z = 1;
	LoadData();
}

TerrainHeightmapDialog::~TerrainHeightmapDialog()
{
	Unsubscribe(data);
}

void TerrainHeightmapDialog::ApplyData()
{
}



void TerrainHeightmapDialog::OnSizeChange()
{
	stretch_x = GetFloat("stretch_x");
	stretch_z = GetFloat("stretch_z");
	Redraw("preview");
}



void TerrainHeightmapDialog::OnFindFilter()
{
	if (ed->FileDialog(FDTexture, false, false)){
		filter_file = ed->DialogFileComplete;
		SetString("filter_image", ed->DialogFile);
		filter.Load(filter_file);
		Redraw("preview");
	}
}



void TerrainHeightmapDialog::OnUpdate(Observable *o)
{
}



void TerrainHeightmapDialog::OnFindHeightmap()
{
	if (ed->FileDialog(FDTexture, false, false)){
		heightmap_file = ed->DialogFileComplete;
		SetString("height_image", ed->DialogFile);
		heightmap.Load(heightmap_file);
		Redraw("preview");
		Enable("ok", true);
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
	return c2f(im.GetPixelInterpolated(x, y));
}

void TerrainHeightmapDialog::OnPreviewDraw()
{
	if (heightmap.Empty())
		return;
	HuiDrawingContext *c = BeginDraw("preview");
	Image m;
	int w = c->width, h = c->height;
	m.Create(w, h, White);
	for (int x=0;x<w;x++)
		for (int y=0;y<h;y++){
			float hmx = (float)x / (float)w;
			float hmy = (float)y / (float)h;
			float f = im_interpolate(heightmap, hmx, hmy, stretch_x, stretch_z);
			if (!filter.Empty())
				f *= im_interpolate(filter, hmx, hmy, 1, 1);
			m.SetPixel(x, y, color(1, f, f, f));
		}

	c->DrawImage(0, 0, m);
	c->End();
}



void TerrainHeightmapDialog::OnOk()
{
	float height_factor = GetFloat("height_factor");
	bool additive = IsChecked("height_op:add");
	data->Execute(new ActionWorldTerrainApplyHeightmap(data, heightmap_file, height_factor, stretch_x, stretch_z, filter_file));//, additive));
	delete(this);
}



void TerrainHeightmapDialog::OnClose()
{
	delete(this);
}



void TerrainHeightmapDialog::LoadData()
{
	SetFloat("stretch_x", stretch_x);
	SetFloat("stretch_z", stretch_z);
	SetFloat("height_factor", 100);
	Check("height_op:set", true);
}


