/*----------------------------------------------------------------------------*\
| Meta                                                                         |
| -> administration of animations, models, items, materials etc                |
| -> centralization of game data                                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
|  - paths get completed with the belonging root-directory of the file type    |
|    (model,item)                                                              |
|  - independent models                                                        |
|     -> equal loading commands create new instances                           |
|     -> equal loading commands copy existing models                           |
|         -> databases of original and copied models                           |
|         -> some data is referenced (skin...)                                 |
|         -> additionally created: effects, absolute physical data,...         |
|     -> each object has its own model                                         |
|  - independent items (managed by CMeta)                                      |
|     -> new items additionally saved as an "original item"                    |
|     -> an array of pointers points to each item                              |
|     -> each item has its unique ID (index in the array) for networking       |
|  - materials stay alive forever, just one instance                           |
|                                                                              |
| last updated: 2009.12.09 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "font.h"
#include "../lib/nix/nix.h"

string FontDir;

// fonts
Array<XFont*> XFonts;



void FontInit()
{}

void FontEnd()
{
	FontReset();
}

void FontReset()
{}

static string str_utf8_to_ubyte(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++)
		if ((((unsigned int)str[i] & 0x80) > 0) && (i < str.num - 1)){
			r.add(((str[i] & 0x1f) << 6) + (str[i + 1] & 0x3f));
			i ++;
		}else
			r.add(str[i]);
	return r;
}

int _cdecl LoadFont(const string &filename)
{
	// "" -> default font
	if (filename.num == 0)
		return 0;

	foreachi(XFont *ff, XFonts, i)
		if (ff->filename  == filename.sys_filename())
			return i;
	CFile *f = FileOpen(FontDir + filename + ".xfont");
	if (!f)
		return -1;
	int ffv=f->ReadFileFormatVersion();
	if (ffv==2){
		XFont *font = new XFont;
		font->filename = filename.sys_filename();
		font->texture = NixLoadTexture(f->ReadStrC());
		int tx = 1;
		int ty = 1;
		if (font->texture >= 0){
			tx = NixTextures[font->texture].width;
			ty = NixTextures[font->texture].height;
		}
		int num_glyphs = f->ReadWordC();
		int height=f->ReadByteC();
		int y1=f->ReadByteC();
		int y2=f->ReadByteC();
		float dy=float(y2-y1);
		font->height=(float)height/dy;
		font->y_offset=(float)y1/dy;
		font->x_factor=(float)f->ReadByteC()*0.01f;
		font->y_factor=(float)f->ReadByteC()*0.01f;
		f->ReadComment();
		int x=0,y=0;
		for (int i=0;i<num_glyphs;i++){
			string name = f->ReadStr();
			int c = (unsigned char)str_utf8_to_ubyte(name)[0];
			int w=f->ReadByte();
			int x1=f->ReadByte();
			int x2=f->ReadByte();
			if (x+w>tx){
				x=0;
				y+=height;
			}
			XGlyph g;
			g.x_offset = (float)x1/dy;
			g.width = (float)w/dy;
			g.dx = (float)(x2-x1)/dy;
			g.dx2 = (float)(w-x1)/dy;
			g.src = rect(	(float)(x+0.5f)/(float)tx,
											(float)(x+0.5f+w)/(float)tx,
											(float)y/(float)ty,
											(float)(y+height)/(float)ty);
			font->glyph[c] = g;
			x+=w;
		}
		/*int u=(unsigned char)f->ReadStrC()[0];
		font->unknown_glyph_no = font->table[u];
		if (font->unknown_glyph_no<0)
			font->unknown_glyph_no=0;
		for (int i=0;i<256;i++)
			if (font->table[i] < 0)
				font->table[i] = font->unknown_glyph_no;*/
		XFonts.add(font);
	}else{
		msg_error(format("wrong file format: %d (expected: 2)",ffv));
	}
	FileClose(f);

	return XFonts.num-1;
}

// retrieve the width of a given text
float _cdecl XFGetWidth(float height, const string &str, int font)
{
	XFont *f = XFonts[font];
	if (!f)
		return 0;
	float w = 0;
	float xf = height * f->x_factor;
	string s = str_utf8_to_ubyte(str);
	for (int i=0;i<s.num;i++){
		int n = (unsigned char)s[i];
		w += f->glyph[n].dx * xf;
	}
	return w;
}

// display a string with our font (values relative to screen)
float _cdecl XFDrawStr(float x, float y, float z, float height, const string &str, int font, bool centric)
{
	XFont *f = XFonts[font];
	if (!f)
		return 0;
	msg_db_r("XFDrawStr",10);
	if (centric)
		x -= XFGetWidth(height, str, font) / 2;
	NixSetAlpha(AlphaSourceAlpha,AlphaSourceInvAlpha);
		//NixSetAlpha(AlphaMaterial);
	NixSetTexture(f->texture);
	float xf=height*f->x_factor * 1.33f;
	float yf=height*f->y_factor;
	float w=0;
	y-=f->y_offset*yf;
	rect d;
	string s = str_utf8_to_ubyte(str);
	for (int i=0;i<s.num;i++){
		if (s[i]=='\r')
			continue;
		if (s[i]=='\n'){
			w=0;
			y+=f->height*yf;
			continue;
		}
		int n=(unsigned char)s[i];
		d.x1=(x+w-f->glyph[n].x_offset*xf);
		d.x2=(x+w+f->glyph[n].dx2     *xf);
		d.y1=(y             );
		d.y2=(y+f->height*yf);
		NixDraw2D(f->glyph[n].src, d, z);
		w += f->glyph[n].dx * xf;
	}
	NixSetAlpha(AlphaNone);
	msg_db_l(10);
	return w;
}

// vertically display a text 
float _cdecl XFDrawVertStr(float x, float y, float z, float height, const string &str, int font)
{
	XFont *f = XFonts[font];
	if (!f)
		return 0;
	msg_db_r("XFDrawVertStr",10);
	NixSetAlpha(AlphaSourceAlpha,AlphaSourceInvAlpha);
	NixSetTexture(f->texture);
	float xf=height*f->x_factor;
	float yf=height*f->y_factor;
	y-=f->y_offset*yf;
	rect d;
	string s = str_utf8_to_ubyte(str);
	for (int i=0;i<s.num;i++){
		if (s[i]=='\r')
			continue;
		if (s[i]=='\n')
			continue;
		int n=(unsigned char)s[i];
		d.x1=(x-f->glyph[n].x_offset*xf);
		d.x2=(x+f->glyph[n].dx2     *xf);
		d.y1=(y             );
		d.y2=(y+f->height*yf);
		NixDraw2D(f->glyph[n].src, d, z);
		y+=yf*0.8f;
	}
	NixSetAlpha(AlphaNone);
	msg_db_l(10);
	return 0;
}

