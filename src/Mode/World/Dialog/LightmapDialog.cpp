/*
 * LightmapDialog.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "LightmapDialog.h"
#include "../../../Data/World/Lightmap/LightmapData.h"
#include "../../../Data/World/Lightmap/LightmapPhotonMap.h"

LightmapDialog::LightmapDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("lightmap_dialog");

	EventM("cancel", this, &LightmapDialog::OnClose);
	EventM("hui:close", this, &LightmapDialog::OnClose);
	EventM("ok", this, &LightmapDialog::OnOk);
	EventM("preview", this, &LightmapDialog::OnPreview);

	//LoadData();
	SetFloat("brightness", 10.0f);
	SetFloat("exponent", 1.0f);
	SetInt("photons", 5000);

	lmd = new LightmapData(data);

	foreach(LightmapData::Model &m, lmd->Models)
		AddString("lightmap_list", m.orig_name);
	Enable("ok", lmd->Models.num > 0);
}

LightmapDialog::~LightmapDialog()
{
	delete(lmd);
}

void LightmapDialog::OnClose()
{
	delete(this);
}

void LightmapDialog::SetData()
{
	lmd->emissive_brightness = GetFloat("brightness");
	lmd->color_exponent = GetFloat("exponent");
	lmd->allow_sun = IsChecked("allow_sun");
}

static Lightmap::Histogram *hist_p;

void OnHistDraw()
{
	HuiDrawingContext *c = HuiCurWindow->BeginDraw("area");
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
	float d = pow(10, floor(log10(grid_dist_min)) + 1);
	if (d > grid_dist_min * 2)
		d /= 2;
	c->SetColor(Grey);
	for (float x=0; x<hist_p->max; x+=d){
		c->DrawStr(scale * x, hh + 3, f2s(x, 2));
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

void ShowHistogram(Lightmap::Histogram &h, CHuiWindow *root)
{
	hist_p = &h;
	CHuiWindow *dlg = HuiCreateSizableDialog("Histogram", 400, 300, root, false);
	dlg->AddControlTable("", 0, 0, 1, 2, "table");
	dlg->SetTarget("table", 0);
	dlg->AddDrawingArea("", 0, 0, 0, 0, "area");
	dlg->AddButton(_("Schlie&sen"), 0, 1, 0, 0, "close");
	dlg->SetImage("close", "hui:close");
	dlg->EventX("area", "hui:redraw", &OnHistDraw);
	dlg->Event("hui:close", &OnHistClose);
	dlg->Event("close", &OnHistClose);
	dlg->Update();

	HuiWaitTillWindowClosed(dlg);
}

void LightmapDialog::OnPreview()
{
	SetData();
	Lightmap *lm = new LightmapPhotonMap(lmd, GetInt("photons"));
	Lightmap::Histogram h = lm->Preview();
	ShowHistogram(h, this);
	delete(lm);
}

void LightmapDialog::OnOk()
{
	SetData();
	Lightmap *lm = new LightmapPhotonMap(lmd, GetInt("photons"));
	lm->Create();
	delete(lm);
	delete(this);
}

