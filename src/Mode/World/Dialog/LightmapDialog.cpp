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
}

LightmapDialog::~LightmapDialog()
{
}

void LightmapDialog::OnClose()
{
	delete(this);
}

void LightmapDialog::OnOk()
{
	LightmapData *lmd = new LightmapData(data);
	msg_write("--------------------");
	msg_write(lmd->Lights.num);
	msg_write(lmd->Trias.num);
	Lightmap *lm = new LightmapPhotonMap(lmd, 2000);
	Lightmap::Histogram h = lm->Preview();
	msg_write(f2s(h.max, 3));
	msg_write(fa2s(h.f));
	delete(lm);
	delete(lmd);
}

