/*
 * DataFont.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAFONT_H_
#define DATAFONT_H_

#include "../Data.h"

namespace Gui{
	class Font;
}
namespace nix{
	class Texture;
};


class DataFont: public Data
{
public:
	DataFont();
	virtual ~DataFont();

	void reset();
	bool load(const string &_filename, bool deep = true);
	bool save(const string &_filename);

	void UpdateTexture();
	void ApplyFont(Gui::Font *f);

	// properties
	struct GlobalData
	{
		string TextureFile;

		int UnknownGlyphNo;
		int GlyphHeight, GlyphY1, GlyphY2;
		int XFactor, YFactor;

		void Reset();
	};
	GlobalData global;

	struct Glyph{
		string Name;
		int Width, X1, X2;
	};
	Array<Glyph> glyph;

	// for editing
	nix::Texture *Texture;
	int TextureWidth, TextureHeight;
	int Marked;

	// actions
	void EditGlobal(const GlobalData &new_data);
	void EditGlyph(int index, const Glyph &new_glyph);
};

#endif /* DATAFONT_H_ */
