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

	//LoadData();
	SetFloat("brightness", 1.0f);
	SetInt("photons", 5000);
}

LightmapDialog::~LightmapDialog()
{
}

void LightmapDialog::OnClose()
{
	delete(this);
}

static Lightmap::Histogram *hist_p;

void OnHistDraw()
{
	HuiDrawingContext *c = HuiCurWindow->BeginDraw("area");
	float w = c->width;
	float h = c->height;
	c->SetColor(White);
	c->DrawRect(0, 0, w, h);
	c->SetColor(Black);
	for (int i=0;i<hist_p->f.num-1;i++){
		c->DrawLine((w * i) / hist_p->f.num, h - h * hist_p->f[i], (w * (i + 1)) / hist_p->f.num, h - h * hist_p->f[i + 1]);
	}
	c->DrawStr(10, 10, f2s(hist_p->max, 3));
	c->End();
}

void OnHistClose()
{
	delete(HuiCurWindow);
}

void ShowHistogram(Lightmap::Histogram &h, CHuiWindow *root)
{
	hist_p = &h;
	CHuiWindow *dlg = HuiCreateDialog("Histogram", 400, 300, root, false);
	dlg->AddDrawingArea("", 5, 5, 390, 290, "area");
	dlg->EventX("area", "hui:redraw", &OnHistDraw);
	dlg->Event("hui:close", &OnHistClose);
	dlg->Update();

	HuiWaitTillWindowClosed(dlg);
}

void LightmapDialog::OnOk()
{
	LightmapData *lmd = new LightmapData(data);
	lmd->emissive_brightness = GetFloat("brightness");
	lmd->allow_sun = IsChecked("allow_sun");
	Lightmap *lm = new LightmapPhotonMap(lmd, GetInt("photons"));
	Lightmap::Histogram h = lm->Preview();
	ShowHistogram(h, this);
	delete(lm);
	delete(lmd);
}

