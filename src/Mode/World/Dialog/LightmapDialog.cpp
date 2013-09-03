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

	EventM("cancel", this, &LightmapDialog::OnClose);
	EventM("hui:close", this, &LightmapDialog::OnClose);
	EventM("ok", this, &LightmapDialog::OnOk);
	EventM("preview", this, &LightmapDialog::OnPreview);
	EventM("resolution", this, &LightmapDialog::OnResolution);
	EventM("lightmap_type", this, &LightmapDialog::OnType);
	EventM("find_new_world", this, &LightmapDialog::OnFindNewWorld);

	//LoadData();
	SetFloat("brightness", 10.0f);
	SetFloat("exponent", 0.8f);
	SetInt("photons", 500000);
	SetInt("lightmap_type", 4);
	Enable("photons", true);
	//SetString("new_world_name", data->filename.basename().replace(".world", "") + "Lightmap");
	SetString("new_world_name", "temp");

	lmd = new LightmapData(data);

	SetFloat("resolution", lmd->resolution);
	Check("allow_sun", lmd->allow_sun);

	FillList();
	Enable("ok", lmd->Models.num > 0);
}

LightmapDialog::~LightmapDialog()
{
	delete(lmd);
}

void LightmapDialog::FillList()
{
	Reset("lightmap_list");
	foreach(LightmapData::Model &m, lmd->Models)
		AddString("lightmap_list", m.orig_name + format("\\%dx%d\\%f", m.tex_width, m.tex_height, sqrt(m.area) / m.tex_width));
}

void LightmapDialog::OnClose()
{
	delete(this);
}

void LightmapDialog::OnType()
{
	Enable("photons", (GetInt("lightmap_type") == 3));
}

void LightmapDialog::SetData()
{
	string new_world = GetString("new_world_name");
	lmd->emissive_brightness = GetFloat("brightness");
	lmd->color_exponent = GetFloat("exponent");
	lmd->allow_sun = IsChecked("allow_sun");
	lmd->texture_out_dir = "Lightmap/" + new_world + "/";
	lmd->model_out_dir = "Lightmap/" + new_world + "/";
}

static Lightmap::Histogram *hist_p;

void OnHistDraw()
{
	HuiPainter *c = HuiCurWindow->BeginDraw("area");
	c->SetFontSize(10);
	float w = c->width;
	float h = c->height;
	float hh = h - 40;
	float scale = w / hist_p->max;
	c->SetColor(White);
	c->DrawRect(0, 0, w, hh);
	c->SetColor(Black);
	c->SetLineWidth(0.8f);
	c->DrawLine(0, hh, w, hh);
	float grid_dist_min = 40 / scale; // 40 pixel
	int dec = floor(log10(grid_dist_min)) + 1;
	float d = pow(10.0f, (float)dec);
	if (d > grid_dist_min * 2)
		d /= 2;
	c->SetColor(Grey);
	for (float x=0; x<hist_p->max; x+=d){
		c->DrawStr(scale * x, hh + 3, f2s(x, max(0, 1-dec)));
		c->DrawLine(scale * x, 0, scale * x, hh);
	}
	c->SetColor(Black);
	c->SetFont("Sans", 12, true, false);
	c->DrawStr(w / 2 - 40, hh + 20, _("Helligkeit"));
	c->SetLineWidth(1.5f);
	for (int i=0;i<hist_p->f.num-1;i++)
		c->DrawLine((w * i) / hist_p->f.num, hh - hh * hist_p->f[i], (w * (i + 1)) / hist_p->f.num, hh - hh * hist_p->f[i + 1]);
	c->End();
}

void OnHistClose()
{
	delete(HuiCurWindow);
}

void ShowHistogram(Lightmap::Histogram &h, HuiWindow *root)
{
	hist_p = &h;
	HuiWindow *dlg = new HuiDialog("Histogram", 400, 300, root, false);
	dlg->AddControlTable("", 0, 0, 1, 2, "table");
	dlg->SetTarget("table", 0);
	dlg->AddDrawingArea("", 0, 0, 0, 0, "area");
	dlg->AddButton(_("Schlie&sen"), 0, 1, 0, 0, "close");
	dlg->SetImage("close", "hui:close");
	dlg->EventX("area", "hui:redraw", &OnHistDraw);
	dlg->Event("hui:close", &OnHistClose);
	dlg->Event("close", &OnHistClose);
	dlg->Run();
}

void LightmapDialog::OnPreview()
{
	SetData();
	Lightmap *lm;
	int type = GetInt("lightmap_type");
	if (type == 4){
		lm = new LightmapPhotonMapImageSpace(lmd, GetInt("photons"));
	}else if (type == 3){
		lm = new LightmapPhotonMap(lmd, GetInt("photons"));
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
	lmd->SetResolution(GetFloat(""));
	FillList();
}

void LightmapDialog::OnFindNewWorld()
{
	if (ed->FileDialog(FDWorld, true, true))
		SetString("new_world_name", ed->DialogFileNoEnding);
}

void LightmapDialog::OnOk()
{
	SetData();
	Lightmap *lm;
	int type = GetInt("lightmap_type");
	if (type == 4){
		lm = new LightmapPhotonMapImageSpace(lmd, GetInt("photons"));
	}else if (type == 3){
		lm = new LightmapPhotonMap(lmd, GetInt("photons"));
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

