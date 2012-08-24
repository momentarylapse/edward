/*
 * DataFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataFont.h"
#include "../../Edward.h"

DataFont::DataFont()
{
	Reset();
}

DataFont::~DataFont()
{
}

bool DataFont::Load(const string & _filename, bool deep)
{
	bool error=false;
	int ffv;
	Reset();

	filename = _filename;
	ed->MakeDirs(filename);
	CFile *f=new CFile();
	if (!f->Open(filename)){
		delete(f);
		ed->SetMessage(_("Kann XFont-Datei nicht &offnen"));
		return false;
	}
	file_time = f->GetDateModification().time;

	ffv=f->ReadFileFormatVersion();
	if (ffv==1){

		TextureFile = f->ReadStrC();
		int NumX=f->ReadByteC();
		int NumY=f->ReadByteC();
		int MaxGlyphWidth=f->ReadByteC();
		GlyphHeight=f->ReadByteC();
		GlyphY1=f->ReadByteC();
		GlyphY2=f->ReadByteC();
		f->ReadByteC(); // XOffset
		XFactor=f->ReadByteC();
		YFactor=f->ReadByteC();
		f->ReadComment();
		Glyph.resize(NumX*NumY);
		for (int i=0;i<NumX*NumY;i++){
			Glyph[i].Name = f->ReadStr();
			Glyph[i].Width = MaxGlyphWidth;
			Glyph[i].X2 = f->ReadByte();
			Glyph[i].X1 = f->ReadByte();
		}
		string str = f->ReadStrC();
		UnknownGlyphNo=0;
		foreachi(Glyph, g, i)
			if (g.Name == str)
				UnknownGlyphNo = i;

	}else if (ffv==2){

		TextureFile = f->ReadStrC();
		int NumGlyphs=f->ReadWordC();
		GlyphHeight=f->ReadByteC();
		GlyphY1=f->ReadByteC();
		GlyphY2=f->ReadByteC();
		XFactor=f->ReadByteC();
		YFactor=f->ReadByteC();
		f->ReadComment();
		Glyph.resize(NumGlyphs);
		for (int i=0;i<NumGlyphs;i++){
			Glyph[i].Name = f->ReadStr();
			Glyph[i].Width=f->ReadByte();
			Glyph[i].X1=f->ReadByte();
			Glyph[i].X2=f->ReadByte();
		}
		string str = f->ReadStrC();
		UnknownGlyphNo=0;
		foreachi(Glyph, g, i)
			if (g.Name == str)
				UnknownGlyphNo = i;
	}else{
		ed->SetMessage(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 1, 2));
		error=true;
	}

	f->Close();
	delete(f);

	if (deep){
		Texture = NixLoadTexture(TextureFile);
		TextureWidth = NixTexture[Texture].Width;
		TextureHeight = NixTexture[Texture].Height;
//		SetFont();
	}

	ResetHistory();
	return !error;
}



// default character set
string PreGlyphName[256]={
	"A",	"B",	"C",	"D",	"E",	"F",	"G",	"H",	"I",	"J",	"K",	"L",	"M",
	"N",	"O",	"P",	"Q",	"R",	"S",	"T",	"U",	"V",	"W",	"X",	"Y",	"Z",
	"a",	"b",	"c",	"d",	"e",	"f",	"g",	"h",	"i",	"j",	"k",	"l",	"m",
	"n",	"o",	"p",	"q",	"r",	"s",	"t",	"u",	"v",	"w",	"x",	"y",	"z",
	"0",	"1",	"2",	"3",	"4",	"5",	"6",	"7",	"8",	"9",
	"&A",	"&O",	"&U",	"&a",	"&o",	"&u",	"&s",
	",",	".",	":",	";",	"!",	"?",	"+",	"-",	"*",	"(",	")",	"/",	"|",
	"\\",	"&",	"\"",	"<",	">",	"=",	"[",	"]",	"%",	"#",	"@",	"§",	"$",
	"~",	"°",	"^",	"'",	" ",	"_",
	"" // "" = end of list
};


void DataFont::Reset()
{
	filename = "";
	TextureFile = "";
	Texture=-1;
	TextureWidth=512;
	TextureHeight=256;
	Glyph.clear();
	GlyphHeight=25;
	GlyphY1=5;
	GlyphY2=20;
	XFactor=YFactor=100;
	UnknownGlyphNo=0;
	for (int i=0;i<256;i++){
		FontGlyph g;
		g.Name = PreGlyphName[i];
		g.Width = 20;
		g.X1 = 3;
		g.X2 = 17;
		if (PreGlyphName[i] == "?")
			UnknownGlyphNo = i;
		Glyph.add(g);
		if (PreGlyphName[i+1].num == 0)
			break;
	};

	ResetHistory();
//	SetFont();
}



bool DataFont::Save(const string & _filename)
{
	return false;
}


