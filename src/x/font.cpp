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
#include "../meta.h"

namespace Gui
{

Path FontDir;



// fonts
Array<Font*> Fonts;



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

Font *_cdecl LoadFont(const Path &filename)
{
	// "" -> default font
	if (filename.is_empty())
		return Fonts[0];

	foreachi(Font *ff, Fonts, i)
		if (ff->filename  == filename)
			return ff;
	File *f = FileOpenText(FontDir << filename.with(".xfont"));
	if (!f)
		return engine.default_font;
	int ffv=f->ReadFileFormatVersion();
	if (ffv==2){
		Font *font = new Font;
		font->filename = filename;
		f->read_comment();
		font->texture = nix::LoadTexture(f->read_str());
		int tx = font->texture->width;
		int ty = font->texture->height;
		f->read_comment();
		int num_glyphs = f->read_word();
		f->read_comment();
		int height = f->read_byte();
		f->read_comment();
		int y1 = f->read_byte();
		f->read_comment();
		int y2 = f->read_byte();
		float dy = float(y2-y1);
		font->height = (float)height/dy;
		font->y_offset = (float)y1/dy;
		f->read_comment();
		font->x_factor = (float)f->read_byte()*0.01f;
		f->read_comment();
		font->y_factor = (float)f->read_byte()*0.01f;
		f->read_comment();
		int x = 0, y = 0;
		for (int i=0;i<num_glyphs;i++){
			string name = f->read_str();
			int c = 0;
			if (name.num > 0)
				c = (unsigned char)str_utf8_to_ubyte(name)[0];
			int w = f->read_byte();
			int x1 = f->read_byte();
			int x2 = f->read_byte();
			if (x+w>tx){
				x = 0;
				y += height;
			}
			Glyph g;
			g.x_offset = (float)x1/dy;
			g.width = (float)w/dy;
			g.dx = (float)(x2-x1)/dy;
			g.dx2 = (float)(w-x1)/dy;
			g.src = rect(	(float)(x+0.5f)/(float)tx,
											(float)(x+0.5f+w)/(float)tx,
											(float)y/(float)ty,
											(float)(y+height)/(float)ty);
			font->glyph[c] = g;
			x += w;
		}
		/*int u=(unsigned char)f->read_strC()[0];
		font->unknown_glyph_no = font->table[u];
		if (font->unknown_glyph_no<0)
			font->unknown_glyph_no=0;
		for (int i=0;i<256;i++)
			if (font->table[i] < 0)
				font->table[i] = font->unknown_glyph_no;*/
		Fonts.add(font);
	}else{
		msg_error(format("wrong file format: %d (expected: 2)",ffv));
	}
	FileClose(f);

	return Fonts.back();
}

// retrieve the width of a given text
float _cdecl Font::getWidth(float height, const string &str)
{
	float w = 0;
	float xf = height * x_factor;
	string s = str_utf8_to_ubyte(str);
	for (int i=0;i<s.num;i++){
		int n = (unsigned char)s[i];
		w += glyph[n].dx * xf;
	}
	return w;
}

// display a string with our font (values relative to screen)
float _cdecl Font::drawStr(float x, float y, float z, float _height, const string &str, bool centric)
{
#if 0
	if (centric)
		x -= getWidth(_height, str) / 2;
	nix::SetAlpha(ALPHA_SOURCE_ALPHA, ALPHA_SOURCE_INV_ALPHA);
	nix::SetTexture(texture);
	float xf=_height*x_factor * 1.33f;
	float yf=_height*y_factor;
	float w=0;
	y-=y_offset*yf;
	rect d;
	string s = str_utf8_to_ubyte(str);
	for (int i=0;i<s.num;i++){
		if (s[i]=='\r')
			continue;
		if (s[i]=='\n'){
			w=0;
			y+=height*yf;
			continue;
		}
		int n=(unsigned char)s[i];
		d.x1=(x+w-glyph[n].x_offset*xf);
		d.x2=(x+w+glyph[n].dx2     *xf);
		d.y1=(y             );
		d.y2=(y+height*yf);
		nix::Draw2D(glyph[n].src, d, z);
		w += glyph[n].dx * xf;
	}
	nix::SetAlpha(ALPHA_NONE);
	return w;
#endif
}

// vertically display a text 
float _cdecl Font::drawStrVert(float x, float y, float z, float _height, const string &str)
{
#if 0
	nix::SetAlpha(ALPHA_SOURCE_ALPHA, ALPHA_SOURCE_INV_ALPHA);
	nix::SetTexture(texture);
	float xf=_height*x_factor;
	float yf=_height*y_factor;
	y-=y_offset*yf;
	rect d;
	string s = str_utf8_to_ubyte(str);
	for (int i=0;i<s.num;i++){
		if (s[i]=='\r')
			continue;
		if (s[i]=='\n')
			continue;
		int n=(unsigned char)s[i];
		d.x1=(x-glyph[n].x_offset*xf);
		d.x2=(x+glyph[n].dx2     *xf);
		d.y1=(y             );
		d.y2=(y+height*yf);
		nix::Draw2D(glyph[n].src, d, z);
		y+=yf*0.8f;
	}
	nix::SetAlpha(ALPHA_NONE);
	return 0;
#endif
}

};

