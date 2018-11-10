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

FontDialog::FontDialog(DataFont *_data) :
	Observer("FontDialog")
{
	from_resource("font_dialog");
	data = _data;
	SettingData = false;


	set_string("text", _("Beispiel Text 0123456789"));

	event("height", std::bind(&FontDialog::OnHeight, this));
	event("y1", std::bind(&FontDialog::OnY1, this));
	event("y2", std::bind(&FontDialog::OnY2, this));
	event("factorx", std::bind(&FontDialog::OnFactorX, this));
	event("factory", std::bind(&FontDialog::OnFactorY, this));
	event("unknown", std::bind(&FontDialog::OnUnknown, this));
	event("name", std::bind(&FontDialog::OnName, this));
	event("x1", std::bind(&FontDialog::OnX1, this));
	event("x2", std::bind(&FontDialog::OnX2, this));
	event("width", std::bind(&FontDialog::OnWidth, this));
	event("text", std::bind(&FontDialog::OnText, this));

	subscribe(data);

	LoadData();
}

FontDialog::~FontDialog()
{
	unsubscribe(data);
}

void FontDialog::LoadData()
{
	SettingData = true;

	global = data->global;
	glyph = data->glyph[data->Marked];

	set_int("num", data->glyph.num);
	set_int("height", global.GlyphHeight);
	set_int("y1", global.GlyphY1);
	set_int("y2", global.GlyphY2);
	set_int("factorx", global.XFactor);
	set_int("factory", global.YFactor);
	set_string("unknown", data->glyph[global.UnknownGlyphNo].Name);

	set_string("name", glyph.Name);
	set_int("x1", glyph.X1);
	set_int("x2", glyph.X2);
	set_int("width", glyph.Width);

	SettingData = false;
}

void FontDialog::OnY1()
{
	global.GlyphY1 = get_int("");
	ApplyGlobalData();
}

void FontDialog::OnY2()
{
	global.GlyphY2 = get_int("");
	ApplyGlobalData();
}

void FontDialog::OnHeight()
{
	global.GlyphHeight = get_int("");
	ApplyGlobalData();
}

void FontDialog::OnFactorX()
{
	global.XFactor = get_int("");
	ApplyGlobalData();
}

void FontDialog::OnFactorY()
{
	global.YFactor = get_int("");
	ApplyGlobalData();
}

void FontDialog::OnUnknown()
{
}

void FontDialog::OnName()
{
	glyph.Name = get_string("");
	ApplyGlyphData();
}

void FontDialog::OnX1()
{
	glyph.X1 = get_int("");
	ApplyGlyphData();
}

void FontDialog::OnX2()
{
	glyph.X2 = get_int("");
	ApplyGlyphData();
}

void FontDialog::OnWidth()
{
	glyph.Width = get_int("");
	ApplyGlyphData();
}

void FontDialog::on_update(Observable *o, const string &message)
{
	LoadData();
}

string FontDialog::GetSampleText()
{
	return get_string("text");
}

void FontDialog::OnText()
{
	ed->force_redraw();
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

