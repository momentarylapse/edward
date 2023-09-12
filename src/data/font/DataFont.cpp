/*
 * DataFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataFont.h"
#include "../../storage/Storage.h"
#include "../../action/font/ActionFontEditGlobal.h"
#include "../../action/font/ActionFontEditGlyph.h"
#include "../../y/Font.h"
#include "../../lib/nix/nix.h"

DataFont::DataFont(Session *s) :
	Data(s, FD_FONT)
{
	reset();
}



// default character set
string PreGlyphName[256]={
	"A",	"B",	"C",	"D",	"E",	"F",	"G",	"H",	"I",	"J",	"K",	"L",	"M",
	"N",	"O",	"P",	"Q",	"R",	"S",	"T",	"U",	"V",	"W",	"X",	"Y",	"Z",
	"a",	"b",	"c",	"d",	"e",	"f",	"g",	"h",	"i",	"j",	"k",	"l",	"m",
	"n",	"o",	"p",	"q",	"r",	"s",	"t",	"u",	"v",	"w",	"x",	"y",	"z",
	"0",	"1",	"2",	"3",	"4",	"5",	"6",	"7",	"8",	"9",
	"\xc3\x84",	"\xc3\x96",	"\xc3\x9c", "\xc3\xa4",	"\xc3\xb6",	"\xc3\xbc",	"\xc3\x9f", // AE, OE, UE, ae, oe, ue, ss
	",",	".",	":",	";",	"!",	"?",	"+",	"-",	"*",	"(",	")",	"/",	"|",
	"\\",	"&",	"\"",	"<",	">",	"=",	"[",	"]",	"%",	"#",	"@",	"\xc2\xa7",	"$", // paragraph
	"~",	"\xc2\xb0",	"^",	"'",	" ",	"_", // degree
	"" // "" = end of list
};


void DataFont::reset() {
	filename = "";
	global.Reset();

	Texture = nullptr;
	TextureWidth = 512;
	TextureHeight = 256;

	Marked = 0;

	glyph.clear();
	for (int i=0;i<256;i++) {
		DataFont::Glyph g;
		g.Name = PreGlyphName[i];
		g.Width = 20;
		g.X1 = 3;
		g.X2 = 17;
		if (PreGlyphName[i] == "?")
			global.UnknownGlyphNo = i;
		glyph.add(g);
		if (PreGlyphName[i+1].num == 0)
			break;
	};

	reset_history();
	out_changed();
//	SetFont();
}



void DataFont::UpdateTexture() {
	Texture = nix::Texture::load(global.TextureFile);
	if (Texture) {
		TextureWidth = Texture->width;
		TextureHeight = Texture->height;
	}
}



int str_utf8_first_char(const string &str) {
	if (str.num == 0)
		return 0;
	if (((unsigned int)str[0] & 0x80) > 0)
		return ((str[0] & 0x1f) << 6) + (str[1] & 0x3f);
	return str[0];
}

void DataFont::ApplyFont(Gui::Font *f) {
	f->texture = Texture;
	f->x_factor = (float)global.XFactor*0.01f;
	f->y_factor = (float)global.YFactor*0.01f;
	float dy = (float)(global.GlyphY2-global.GlyphY1);
	f->y_offset = (float)global.GlyphY1/dy;
	f->height = (float)global.GlyphHeight/dy;
	int x = 0, y = 0;
	foreachi(Glyph &g, glyph, i) {
		if (x + g.Width > TextureWidth){
			x = 0;
			y += global.GlyphHeight;
		}
		int c = str_utf8_first_char(g.Name) & 0xff;
		f->glyph[c].x_offset = (float)g.X1 / dy;
		f->glyph[c].width = (float)g.Width / dy;
		f->glyph[c].dx = (float)(g.X2 - g.X1) / dy;
		f->glyph[c].dx2 = (float)(g.Width - g.X1) / dy;
		f->glyph[c].src = rect(	(float)(x - 0.5f) / (float)TextureWidth,
								(float)(x - 0.5f + g.Width) / (float)TextureWidth,
								(float)y / (float)TextureHeight,
								(float)(y + global.GlyphHeight) / (float)TextureHeight);
		x += g.Width;
	}
}

void DataFont::EditGlobal(const GlobalData &new_data) {
	execute(new ActionFontEditGlobal(new_data));
}

void DataFont::EditGlyph(int index, const Glyph &new_glyph) {
	execute(new ActionFontEditGlyph(index, new_glyph));
}


void DataFont::GlobalData::Reset() {
	TextureFile = "";
	GlyphHeight=25;
	GlyphY1=5;
	GlyphY2=20;
	XFactor=YFactor=100;
	UnknownGlyphNo=0;
}

