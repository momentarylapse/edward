/*
 * LightmapDialog.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "LightmapDialog.h"
#include "../../../Data/World/Lightmap/LightmapData.h"
#include "../../../Data/World/Lightmap/LightmapPhotonMap.h"
#include "../../../Data/World/Lightmap/LightmapPhotonMapImageSpace.h"
#include "../../../Data/World/Lightmap/LightmapRayTracing.h"
#include "../../../Data/World/Lightmap/LightmapRadiosity.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../x/model_manager.h"
#include "../../../Edward.h"

LightmapDialog::LightmapDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	HuiWindow("lightmap_dialog", _parent, _allow_parent)
{
	data = _data;

	event("cancel", this, &LightmapDialog::OnClose);
	event("hui:close", this, &LightmapDialog::OnClose);
	event("ok", this, &LightmapDialog::OnOk);
	event("preview", this, &LightmapDialog::OnPreview);
	event("resolution", this, &LightmapDialog::OnResolution);
	event("lightmap_type", this, &LightmapDialog::OnType);
	event("find_new_world", this, &LightmapDialog::OnFindNewWorld);

	//LoadData();
	setFloat("brightness", 10.0f);
	setFloat("exponent", 0.8f);
	setInt("photons", 500000);
	setInt("lightmap_type", 4);
	enable("photons", true);
	//SetString("new_world_name", data->filename.basename().replace(".world", "") + "Lightmap");
	setString("new_world_name", "temp");

	lmd = new LightmapData(data);

	setFloat("resolution", lmd->resolution);
	check("allow_sun", lmd->allow_sun);

	FillList();
	enable("ok", lmd->Models.num > 0);
}

LightmapDialog::~LightmapDialog()
{
	delete(lmd);
}

void LightmapDialog::FillList()
{
	reset("lightmap_list");
	foreach(LightmapData::Model &m, lmd->Models)
		addString("lightmap_list", m.orig_name + format("\\%dx%d\\%f", m.tex_width, m.tex_height, sqrt(m.area) / m.tex_width));
	foreach(LightmapData::Terrain &t, lmd->Terrains)
		addString("lightmap_list", t.orig_name + format("\\%dx%d\\%f", t.tex_width, t.tex_height, sqrt(t.area) / t.tex_width));
}

void LightmapDialog::OnClose()
{
	delete(this);
}

void LightmapDialog::OnType()
{
	enable("photons", (getInt("lightmap_type") == 3));
}

void LightmapDialog::SetData()
{
	lmd->new_world_name = getString("new_world_name");
	lmd->emissive_brightness = getFloat("brightness");
	lmd->color_exponent = getFloat("exponent");
	lmd->allow_sun = isChecked("allow_sun");
	lmd->texture_out_dir = "Lightmap/" + lmd->new_world_name + "/";
	lmd->model_out_dir = "Lightmap/" + lmd->new_world_name + "/";
}

static Lightmap::Histogram *hist_p;

void OnHistDraw()
{
	HuiPainter *c = HuiCurWindow->beginDraw("area");
	c->setFontSize(10);
	float w = c->width;
	float h = c->height;
	float hh = h - 40;
	float scale = w / hist_p->max;
	c->setColor(White);
	c->drawRect(0, 0, w, hh);
	c->setColor(Black);
	c->setLineWidth(0.8f);
	c->drawLine(0, hh, w, hh);
	float grid_dist_min = 40 / scale; // 40 pixel
	int dec = floor(log10(grid_dist_min)) + 1;
	float d = pow(10.0f, (float)dec);
	if (d > grid_dist_min * 2)
		d /= 2;
	c->setColor(Grey);
	for (float x=0; x<hist_p->max; x+=d){
		c->drawStr(scale * x, hh + 3, f2s(x, max(0, 1-dec)));
		c->drawLine(scale * x, 0, scale * x, hh);
	}
	c->setColor(Black);
	c->setFont("Sans", 12, true, false);
	c->drawStr(w / 2 - 40, hh + 20, _("Helligkeit"));
	c->setLineWidth(1.5f);
	for (int i=0;i<hist_p->f.num-1;i++)
		c->drawLine((w * i) / hist_p->f.num, hh - hh * hist_p->f[i], (w * (i + 1)) / hist_p->f.num, hh - hh * hist_p->f[i + 1]);
	c->end();
}

void OnHistClose()
{
	delete(HuiCurWindow);
}

void ShowHistogram(Lightmap::Histogram &h, HuiWindow *root)
{
	hist_p = &h;
	HuiWindow *dlg = new HuiDialog("Histogram", 400, 300, root, false);
	dlg->addControlTable("", 0, 0, 1, 2, "table");
	dlg->setTarget("table", 0);
	dlg->addDrawingArea("", 0, 0, 0, 0, "area");
	dlg->addButton(_("Schlie&sen"), 0, 1, 0, 0, "close");
	dlg->setImage("close", "hui:close");
	dlg->eventSX("area", "hui:draw", &OnHistDraw);
	dlg->eventS("hui:close", &OnHistClose);
	dlg->eventS("close", &OnHistClose);
	dlg->run();
}

void LightmapDialog::OnPreview()
{
	SetData();
	Lightmap *lm;
	int type = getInt("lightmap_type");
	if (type == 4){
		lm = new LightmapPhotonMapImageSpace(lmd, getInt("photons"));
	}else if (type == 3){
		lm = new LightmapPhotonMap(lmd, getInt("photons"));
	}else if ((type == 1) || (type == 2)){
		lm = new LightmapRadiosity(lmd);
	}else{
		lm = new LightmapRayTracing(lmd);
	}
	if (lm->Preview()){
		Lightmap::Histogram h = lm->GetHistogram();
		ShowHistogram(h, this);
	}
	delete(lm);
}

void LightmapDialog::OnResolution()
{
	lmd->SetResolution(getFloat(""));
	FillList();
}

void LightmapDialog::OnFindNewWorld()
{
	if (ed->fileDialog(FD_WORLD, true, true))
		setString("new_world_name", ed->dialog_file_no_ending);
}

void LightmapDialog::OnOk()
{
	SetData();
	Lightmap *lm;
	int type = getInt("lightmap_type");
	if (type == 4){
		lm = new LightmapPhotonMapImageSpace(lmd, getInt("photons"));
	}else if (type == 3){
		lm = new LightmapPhotonMap(lmd, getInt("photons"));
	}else if ((type == 1) || (type == 2)){
		lm = new LightmapRadiosity(lmd);
	}else{
		lm = new LightmapRayTracing(lmd);
	}
	bool ok = lm->Create();
	delete(lm);
	if (ok)
		delete(this);
}

