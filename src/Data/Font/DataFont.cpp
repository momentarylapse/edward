/*
 * DataFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataFont.h"
#include "../../Edward.h"
#include "../../Action/Font/ActionFontEditGlobal.h"
#include "../../Action/Font/ActionFontEditGlyph.h"
#include "../../x/font.h"
#include "../../lib/nix/nix.h"

DataFont::DataFont() :
	Data(FD_FONT)
{
	reset();
}

DataFont::~DataFont()
{
}

bool DataFont::load(const string & _filename, bool deep)
{
	bool error=false;
	int ffv;
	reset();

	filename = _filename;
	ed->make_dirs(filename);
	File *f = FileOpenText(filename);
	if (!f){
		ed->set_message(_("Can't load font file"));
		return false;
	}
	file_time = f->GetDateModification().time;

	ffv = f->ReadFileFormatVersion();
	if (ffv == 1){

		f->read_comment();
		global.TextureFile = f->read_str();
		f->read_comment();
		int NumX = f->read_byte();
		f->read_comment();
		int NumY = f->read_byte();
		f->read_comment();
		int MaxGlyphWidth = f->read_byte();
		f->read_comment();
		global.GlyphHeight = f->read_byte();
		f->read_comment();
		global.GlyphY1 = f->read_byte();
		f->read_comment();
		global.GlyphY2 = f->read_byte();
		f->read_comment();
		f->read_byte(); // XOffset
		f->read_comment();
		global.XFactor = f->read_byte();
		f->read_comment();
		global.YFactor = f->read_byte();
		f->read_comment();
		glyph.resize(NumX*NumY);
		for (int i=0;i<NumX*NumY;i++){
			glyph[i].Name = str_m_to_utf8(f->read_str());
			glyph[i].Width = MaxGlyphWidth;
			glyph[i].X2 = f->read_byte();
			glyph[i].X1 = f->read_byte();
		}
		f->read_comment();
		string str = f->read_str();
		global.UnknownGlyphNo = 0;
		foreachi(Glyph &g, glyph, i)
			if (g.Name == str)
				global.UnknownGlyphNo = i;

	}else if (ffv == 2){

		f->read_comment();
		global.TextureFile = f->read_str();
		f->read_comment();
		int NumGlyphs = f->read_word();
		f->read_comment();
		global.GlyphHeight = f->read_byte();
		f->read_comment();
		global.GlyphY1 = f->read_byte();
		f->read_comment();
		global.GlyphY2 = f->read_byte();
		f->read_comment();
		global.XFactor = f->read_byte();
		f->read_comment();
		global.YFactor = f->read_byte();
		f->read_comment();
		glyph.resize(NumGlyphs);
		for (int i=0;i<NumGlyphs;i++){
			glyph[i].Name = str_m_to_utf8(f->read_str());
			glyph[i].Width = f->read_byte();
			glyph[i].X1 = f->read_byte();
			glyph[i].X2 = f->read_byte();
		}
		f->read_comment();
		string str = f->read_str();
		global.UnknownGlyphNo=0;
		foreachi(Glyph &g, glyph, i)
			if (g.Name == str)
				global.UnknownGlyphNo = i;
	}else{
		ed->set_message(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 1, 2));
		error = true;
	}

	delete(f);

	if (deep)
		UpdateTexture();

	reset_history();
	notify();
	return !error;
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


void DataFont::reset()
{
	filename = "";
	global.Reset();

	Texture = NULL;
	TextureWidth = 512;
	TextureHeight = 256;

	Marked = 0;

	glyph.clear();
	for (int i=0;i<256;i++){
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
	notify();
//	SetFont();
}



bool DataFont::save(const string & _filename)
{
	filename = _filename;
	ed->make_dirs(filename);

	File *f = FileCreateText(filename);
	f->WriteFileFormatVersion(false, 2);

	f->write_comment("// Texture");
	f->write_str(global.TextureFile);
	f->write_comment("// Num Glyphs");
	f->write_word(glyph.num);
	f->write_comment("// Glyph Height");
	f->write_byte(global.GlyphHeight);
	f->write_comment("// Glyph Y1");
	f->write_byte(global.GlyphY1);
	f->write_comment("// Glyph Y2");
	f->write_byte(global.GlyphY2);
	f->write_comment("// Scale Factor X");
	f->write_byte(global.XFactor);
	f->write_comment("// Scale Factor Y");
	f->write_byte(global.YFactor);
	f->write_comment("// Glyphs (Char, Width, X1, X2)");
	foreachi(Glyph &g, glyph, i){
		f->write_str(g.Name);
		f->write_byte(g.Width);
		f->write_byte(g.X1);
		f->write_byte(g.X2);
	}
	f->write_comment("// Unknown Char");
	f->write_str(glyph[global.UnknownGlyphNo].Name);

	f->write_str("#");
	delete(f);

	action_manager->mark_current_as_save();
	return false;
}

void DataFont::UpdateTexture()
{
	Texture = nix::LoadTexture(global.TextureFile);
	if (Texture){
		TextureWidth = Texture->width;
		TextureHeight = Texture->height;
	}
}



int str_utf8_first_char(const string &str)
{
	if (str.num == 0)
		return 0;
	if (((unsigned int)str[0] & 0x80) > 0)
		return ((str[0] & 0x1f) << 6) + (str[1] & 0x3f);
	return str[0];
}

void DataFont::ApplyFont(Gui::Font *f)
{
	f->texture = Texture;
	f->x_factor = (float)global.XFactor*0.01f;
	f->y_factor = (float)global.YFactor*0.01f;
	float dy = (float)(global.GlyphY2-global.GlyphY1);
	f->y_offset = (float)global.GlyphY1/dy;
	f->height = (float)global.GlyphHeight/dy;
	int x = 0, y = 0;
	foreachi(Glyph &g, glyph, i){
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

void DataFont::EditGlobal(const GlobalData &new_data)
{	execute(new ActionFontEditGlobal(new_data));	}

void DataFont::EditGlyph(int index, const Glyph &new_glyph)
{	execute(new ActionFontEditGlyph(index, new_glyph));	}


void DataFont::GlobalData::Reset()
{
	TextureFile = "";
	GlyphHeight=25;
	GlyphY1=5;
	GlyphY2=20;
	XFactor=YFactor=100;
	UnknownGlyphNo=0;
}

