/*----------------------------------------------------------------------------*\
| Meta                                                                         |
| -> administration of animations, models, items, materials etc                |
| -> centralization of game data                                               |
|                                                                              |
| last updated: 2009.11.22 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(FONT_H__INCLUDED_)
#define FONT_H__INCLUDED_

#include "../lib/base/base.h"
#include "../lib/math/math.h"

class NixTexture;

struct XGlyph
{
	float dx, dx2, x_offset, width;
	rect src;
};

struct XFont
{
	string filename;
	NixTexture *texture;
	int unknown_glyph_no;
	XGlyph glyph[256];
	float y_offset, height, x_factor, y_factor;
};

void FontInit();
void FontEnd();
void FontReset();

// fonts
int _cdecl LoadFont(const string &filename);
extern string FontDir;

// only used by meta itself and the editor...
extern Array<XFont*> XFonts;


float _cdecl XFGetWidth(float h, const string &str, int font);
float _cdecl XFDrawStr(float x, float y, float z, float height, const string &str, int font, bool centric = false);
float _cdecl XFDrawVertStr(float x, float y, float z, float h, const string &str, int font);

#endif

