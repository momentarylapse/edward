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
	Data(FDFont)
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
	ed->makeDirs(filename);
	CFile *f=new CFile();
	if (!f->Open(filename)){
		delete(f);
		ed->setMessage(_("Kann XFont-Datei nicht &offnen"));
		return false;
	}
	file_time = f->GetDateModification().time;

	ffv=f->ReadFileFormatVersion();
	if (ffv==1){

		global.TextureFile = f->ReadStrC();
		int NumX=f->ReadByteC();
		int NumY=f->ReadByteC();
		int MaxGlyphWidth=f->ReadByteC();
		global.GlyphHeight=f->ReadByteC();
		global.GlyphY1=f->ReadByteC();
		global.GlyphY2=f->ReadByteC();
		f->ReadByteC(); // XOffset
		global.XFactor=f->ReadByteC();
		global.YFactor=f->ReadByteC();
		f->ReadComment();
		glyph.resize(NumX*NumY);
		for (int i=0;i<NumX*NumY;i++){
			glyph[i].Name = str_m_to_utf8(f->ReadStr());
			glyph[i].Width = MaxGlyphWidth;
			glyph[i].X2 = f->ReadByte();
			glyph[i].X1 = f->ReadByte();
		}
		string str = f->ReadStrC();
		global.UnknownGlyphNo=0;
		foreachi(Glyph &g, glyph, i)
			if (g.Name == str)
				global.UnknownGlyphNo = i;

	}else if (ffv==2){

		global.TextureFile = f->ReadStrC();
		int NumGlyphs=f->ReadWordC();
		global.GlyphHeight=f->ReadByteC();
		global.GlyphY1=f->ReadByteC();
		global.GlyphY2=f->ReadByteC();
		global.XFactor=f->ReadByteC();
		global.YFactor=f->ReadByteC();
		f->ReadComment();
		glyph.resize(NumGlyphs);
		for (int i=0;i<NumGlyphs;i++){
			glyph[i].Name = str_m_to_utf8(f->ReadStr());
			glyph[i].Width=f->ReadByte();
			glyph[i].X1=f->ReadByte();
			glyph[i].X2=f->ReadByte();
		}
		string str = f->ReadStrC();
		global.UnknownGlyphNo=0;
		foreachi(Glyph &g, glyph, i)
			if (g.Name == str)
				global.UnknownGlyphNo = i;
	}else{
		ed->setMessage(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 1, 2));
		error=true;
	}

	f->Close();
	delete(f);

	if (deep)
		UpdateTexture();

	resetHistory();
	notify("Change");
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

	resetHistory();
	notify("Change");
//	SetFont();
}



bool DataFont::save(const string & _filename)
{
	filename = _filename;
	ed->makeDirs(filename);

	CFile *f=new CFile();
	if (!f)
		return false;
	f->Create(filename);
	f->WriteFileFormatVersion(false, 2);

	f->WriteComment("// Texture");
	f->WriteStr(global.TextureFile);
	f->WriteComment("// Num Glyphs");
	f->WriteWord(glyph.num);
	f->WriteComment("// Glyph Height");
	f->WriteByte(global.GlyphHeight);
	f->WriteComment("// Glyph Y1");
	f->WriteByte(global.GlyphY1);
	f->WriteComment("// Glyph Y2");
	f->WriteByte(global.GlyphY2);
	f->WriteComment("// Scale Factor X");
	f->WriteByte(global.XFactor);
	f->WriteComment("// Scale Factor Y");
	f->WriteByte(global.YFactor);
	f->WriteComment("// Glyphs (Char, Width, X1, X2)");
	foreachi(Glyph &g, glyph, i){
		f->WriteStr(g.Name);
		f->WriteByte(g.Width);
		f->WriteByte(g.X1);
		f->WriteByte(g.X2);
	}
	f->WriteComment("// Unknown Char");
	f->WriteStr(glyph[global.UnknownGlyphNo].Name);

	f->WriteStr("#");
	f->Close();
	delete(f);

	action_manager->markCurrentAsSave();
	return false;
}

void DataFont::UpdateTexture()
{
	Texture = NixLoadTexture(global.TextureFile);
	if (Texture){
		TextureWidth = Texture->width;
		TextureHeight = Texture->height;
	}
}



int str_utf8_first_ubyte(const string &str)
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
		int c = str_utf8_first_ubyte(g.Name);
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

