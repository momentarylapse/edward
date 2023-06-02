/*
 * DataFont.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAFONT_H_
#define DATAFONT_H_

#include "../Data.h"
#include "../../lib/base/pointer.h"
#include "../../lib/nix/nix.h"

namespace Gui{
	class Font;
}


class DataFont: public Data {
public:
	DataFont();

	void reset();

	void UpdateTexture();
	void ApplyFont(Gui::Font *f);

	// properties
	struct GlobalData {
		Path TextureFile;

		int UnknownGlyphNo;
		int GlyphHeight, GlyphY1, GlyphY2;
		int XFactor, YFactor;

		void Reset();
	};
	GlobalData global;

	struct Glyph {
		string Name;
		int Width, X1, X2;
	};
	Array<Glyph> glyph;

	// for editing
	shared<nix::Texture> Texture;
	int TextureWidth, TextureHeight;
	int Marked;

	// actions
	void EditGlobal(const GlobalData &new_data);
	void EditGlyph(int index, const Glyph &new_glyph);
};

#endif /* DATAFONT_H_ */
