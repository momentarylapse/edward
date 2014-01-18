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

namespace Gui
{

struct Glyph
{
	float dx, dx2, x_offset, width;
	rect src;
};

class Font
{
public:
	string filename;
	NixTexture *texture;
	int unknown_glyph_no;
	Glyph glyph[256];
	float y_offset, height, x_factor, y_factor;

	float _cdecl getWidth(float h, const string &str);
	float _cdecl drawStr(float x, float y, float z, float height, const string &str, bool centric = false);
	float _cdecl drawStrVert(float x, float y, float z, float h, const string &str);
};

void FontInit();
void FontEnd();
void FontReset();

// fonts
Font* _cdecl LoadFont(const string &filename);
extern string FontDir;

// only used by meta itself and the editor...
extern Array<Font*> Fonts;

};

#endif

