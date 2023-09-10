/*
 * FontDialog.cpp
 *
 *  Created on: 20.03.2013
 *      Author: michi
 */

#include "FontDialog.h"
//#include "../../../action/world/ActionWorldEditObject.h"
#include "../../../EdwardWindow.h"
#include "../../../multiview/MultiView.h"


bool SettingDialogData=true;

FontDialog::FontDialog(DataFont *_data) {
	from_resource("font_dialog");
	data = _data;
	SettingData = false;


	set_string("text", _("Example text 0123456789"));

	event("height", [this] { OnHeight(); });
	event("y1", [this] { OnY1(); });
	event("y2", [this] { OnY2(); });
	event("factorx", [this] { OnFactorX(); });
	event("factory", [this] { OnFactorY(); });
	event("unknown", [this] { OnUnknown(); });
	event("name", [this] { OnName(); });
	event("x1", [this] { OnX1(); });
	event("x2", [this] { OnX2(); });
	event("width", [this] { OnWidth(); });
	event("text", [this] { OnText(); });

	data->out_changed >> create_sink([this]{ LoadData(); });

	LoadData();
}

FontDialog::~FontDialog() {
	data->unsubscribe(this);
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

string FontDialog::GetSampleText()
{
	return get_string("text");
}

void FontDialog::OnText()
{
	data->ed->multi_view_2d->force_redraw();
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

