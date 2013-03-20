/*
 * FontDialog.cpp
 *
 *  Created on: 20.03.2013
 *      Author: michi
 */

#include "FontDialog.h"
//#include "../../../Action/World/ActionWorldEditObject.h"
#include "../../../Edward.h"


bool SettingDialogData=true;

FontDialog::FontDialog(CHuiWindow *_parent, bool _allow_parent, DataFont *_data) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;
	SettingData = false;

	// dialog
	FromResource("font_dialog");


	SetString("text", _("Beispiel Text 0123456789"));

	EventM("hui:close", this, &FontDialog::OnClose);
	EventM("height", this, &FontDialog::OnHeight);
	EventM("y1", this, &FontDialog::OnY1);
	EventM("y2", this, &FontDialog::OnY2);
	EventM("factorx", this, &FontDialog::OnFactorX);
	EventM("factory", this, &FontDialog::OnFactorY);
	EventM("unknown", this, &FontDialog::OnUnknown);
	EventM("name", this, &FontDialog::OnName);
	EventM("x1", this, &FontDialog::OnX1);
	EventM("x2", this, &FontDialog::OnX2);
	EventM("width", this, &FontDialog::OnWidth);
	EventM("text", this, &FontDialog::OnText);

	Subscribe(data);

	LoadData();
}

FontDialog::~FontDialog()
{
	Unsubscribe(data);
}

void FontDialog::OnClose()
{
}

void FontDialog::LoadData()
{
	SettingData = true;

	global = data->global;
	glyph = data->glyph[data->Marked];

	SetInt("num", data->glyph.num);
	SetInt("height", global.GlyphHeight);
	SetInt("y1", global.GlyphY1);
	SetInt("y2", global.GlyphY2);
	SetInt("factorx", global.XFactor);
	SetInt("factory", global.YFactor);
	SetString("unknown", data->glyph[global.UnknownGlyphNo].Name);

	SetString("name", glyph.Name);
	SetInt("x1", glyph.X1);
	SetInt("x2", glyph.X2);
	SetInt("width", glyph.Width);

	SettingData = false;
}

void FontDialog::OnY1()
{
	global.GlyphY1 = GetInt("");
	ApplyGlobalData();
}

void FontDialog::OnY2()
{
	global.GlyphY2 = GetInt("");
	ApplyGlobalData();
}

void FontDialog::OnHeight()
{
	global.GlyphHeight = GetInt("");
	ApplyGlobalData();
}

void FontDialog::OnFactorX()
{
	global.XFactor = GetInt("");
	ApplyGlobalData();
}

void FontDialog::OnFactorY()
{
	global.YFactor = GetInt("");
	ApplyGlobalData();
}

void FontDialog::OnUnknown()
{
}

void FontDialog::OnName()
{
	glyph.Name = GetString("");
	ApplyGlyphData();
}

void FontDialog::OnX1()
{
	glyph.X1 = GetInt("");
	ApplyGlyphData();
}

void FontDialog::OnX2()
{
	glyph.X2 = GetInt("");
	ApplyGlyphData();
}

void FontDialog::OnWidth()
{
	glyph.Width = GetInt("");
	ApplyGlyphData();
}

void FontDialog::OnUpdate(Observable *o)
{
	LoadData();
}

string FontDialog::GetSampleText()
{
	return GetString("text");
}

void FontDialog::OnText()
{
	ed->ForceRedraw();
}

void FontDialog::ApplyGlobalData()
{
	if (SettingData)
		return;
	data->EditGlobal(global);
}

void FontDialog::ApplyGlyphData()
{
	if (SettingData)
		return;
	data->EditGlyph(data->Marked, glyph);
}

