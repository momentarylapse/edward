/*
 * TerrainHeightmapDialog.cpp
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#include "TerrainHeightmapDialog.h"
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

	Subscribe(data);

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
	Redraw("preview");
}



void TerrainHeightmapDialog::OnFindFilter()
{
	if (ed->FileDialog(FDTexture, false, false)){
		SetString("filter_image", ed->DialogFileComplete);
		filter.Load(ed->DialogFileComplete);
		Redraw("preview");
	}
}



void TerrainHeightmapDialog::OnUpdate(Observable *o)
{
}



void TerrainHeightmapDialog::OnFindHeightmap()
{
	if (ed->FileDialog(FDTexture, false, false)){
		SetString("height_image", ed->DialogFileComplete);
		heightmap.Load(ed->DialogFileComplete);
		Redraw("preview");
	}
}



void TerrainHeightmapDialog::OnPreviewDraw()
{
	if (heightmap.width == 0)
		return;
	HuiDrawingContext *c = BeginDraw("preview");
	Image m;
	int w = c->width, h = c->height;
	m.Create(w, h, White);
	for (int x=0;x<w;x++)
		for (int y=0;y<h;y++){
			//msg_write(format("%f %f %f %f", (float)x, (float)w, (float)heightmap.width, GetFloat("strech_x")));
			float hmx = loopf((float)x / (float)w * GetFloat("stretch_x"), 0, 1) * (float)heightmap.width;
			float hmy = loopf((float)y / (float)h * GetFloat("stretch_z"), 0, 1) * (float)heightmap.height;
			//msg_write(f2s(hmx, 4));
			float f = heightmap.GetPixel(hmx, hmy).r;
			if (filter.width > 0)
				f *= filter.GetPixel((float)x / (float)w * (float)filter.width, (float)y / (float)h * (float)filter.height).r;
			m.SetPixel(x, y, color(1, f, f, f));
		}

	c->DrawImage(0, 0, m);
	c->End();
}



void TerrainHeightmapDialog::OnOk()
{
	delete(this);
}



void TerrainHeightmapDialog::OnClose()
{
	delete(this);
}



void TerrainHeightmapDialog::LoadData()
{
	SetFloat("stretch_x", 1);
	SetFloat("stretch_z", 1);
	SetFloat("height_factor", 100);
	Check("height_op:set", true);
}


