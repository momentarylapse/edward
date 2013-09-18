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

FontDialog::FontDialog(HuiWindow *_parent, DataFont *_data) :
	EmbeddedDialog(_parent, "font_dialog", "root-table", 1, 0, "noexpandx")
{
	data = _data;
	SettingData = false;


	SetString("text", _("Beispiel Text 0123456789"));

	win->EventM("height", this, &FontDialog::OnHeight);
	win->EventM("y1", this, &FontDialog::OnY1);
	win->EventM("y2", this, &FontDialog::OnY2);
	win->EventM("factorx", this, &FontDialog::OnFactorX);
	win->EventM("factory", this, &FontDialog::OnFactorY);
	win->EventM("unknown", this, &FontDialog::OnUnknown);
	win->EventM("name", this, &FontDialog::OnName);
	win->EventM("x1", this, &FontDialog::OnX1);
	win->EventM("x2", this, &FontDialog::OnX2);
	win->EventM("width", this, &FontDialog::OnWidth);
	win->EventM("text", this, &FontDialog::OnText);

	Subscribe(data);

	LoadData();
}

FontDialog::~FontDialog()
{
	Unsubscribe(data);
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

