/*
 * DataFont.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAFONT_H_
#define DATAFONT_H_

#include "../Data.h"
#include "../../lib/x/x.h"

class XFont;


struct FontGlyph{
	string Name;
	int Width, X1, X2;
};

class DataFont: public Data
{
public:
	DataFont();
	virtual ~DataFont();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);

	void ApplyFont(XFont *f);

	// properties
	int Texture;
	string TextureFile;
	int TextureWidth, TextureHeight;

	int UnknownGlyphNo;
	int GlyphHeight, GlyphY1, GlyphY2;
	int XFactor, YFactor;
	Array<FontGlyph> Glyph;

	// for editing
	int Marked;
};

#endif /* DATAFONT_H_ */
