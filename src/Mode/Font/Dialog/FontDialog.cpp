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


	setString("text", _("Beispiel Text 0123456789"));

	win->event("height", this, &FontDialog::OnHeight);
	win->event("y1", this, &FontDialog::OnY1);
	win->event("y2", this, &FontDialog::OnY2);
	win->event("factorx", this, &FontDialog::OnFactorX);
	win->event("factory", this, &FontDialog::OnFactorY);
	win->event("unknown", this, &FontDialog::OnUnknown);
	win->event("name", this, &FontDialog::OnName);
	win->event("x1", this, &FontDialog::OnX1);
	win->event("x2", this, &FontDialog::OnX2);
	win->event("width", this, &FontDialog::OnWidth);
	win->event("text", this, &FontDialog::OnText);

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

	setInt("num", data->glyph.num);
	setInt("height", global.GlyphHeight);
	setInt("y1", global.GlyphY1);
	setInt("y2", global.GlyphY2);
	setInt("factorx", global.XFactor);
	setInt("factory", global.YFactor);
	setString("unknown", data->glyph[global.UnknownGlyphNo].Name);

	setString("name", glyph.Name);
	setInt("x1", glyph.X1);
	setInt("x2", glyph.X2);
	setInt("width", glyph.Width);

	SettingData = false;
}

void FontDialog::OnY1()
{
	global.GlyphY1 = getInt("");
	ApplyGlobalData();
}

void FontDialog::OnY2()
{
	global.GlyphY2 = getInt("");
	ApplyGlobalData();
}

void FontDialog::OnHeight()
{
	global.GlyphHeight = getInt("");
	ApplyGlobalData();
}

void FontDialog::OnFactorX()
{
	global.XFactor = getInt("");
	ApplyGlobalData();
}

void FontDialog::OnFactorY()
{
	global.YFactor = getInt("");
	ApplyGlobalData();
}

void FontDialog::OnUnknown()
{
}

void FontDialog::OnName()
{
	glyph.Name = getString("");
	ApplyGlyphData();
}

void FontDialog::OnX1()
{
	glyph.X1 = getInt("");
	ApplyGlyphData();
}

void FontDialog::OnX2()
{
	glyph.X2 = getInt("");
	ApplyGlyphData();
}

void FontDialog::OnWidth()
{
	glyph.Width = getInt("");
	ApplyGlyphData();
}

void FontDialog::OnUpdate(Observable *o)
{
	LoadData();
}

string FontDialog::GetSampleText()
{
	return getString("text");
}

void FontDialog::OnText()
{
	ed->forceRedraw();
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

