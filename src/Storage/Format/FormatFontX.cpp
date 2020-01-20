/*
 * FormatFontX.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatFontX.h"
#include "../../Edward.h"

FormatFontX::FormatFontX() : TypedFormat<DataFont>(FD_FONT, "xfont", _("Font"), Flag::CANONICAL_READ_WRITE) {
}

void FormatFontX::_load(const string &filename, DataFont *data, bool deep) {

	int ffv;
	data->reset();

	data->filename = filename;
	ed->make_dirs(filename);
	File *f = FileOpenText(filename);
	data->file_time = f->GetDateModification().time;

	ffv = f->ReadFileFormatVersion();
	if (ffv == 1) {

		f->read_comment();
		data->global.TextureFile = f->read_str();
		f->read_comment();
		int NumX = f->read_byte();
		f->read_comment();
		int NumY = f->read_byte();
		f->read_comment();
		int MaxGlyphWidth = f->read_byte();
		f->read_comment();
		data->global.GlyphHeight = f->read_byte();
		f->read_comment();
		data->global.GlyphY1 = f->read_byte();
		f->read_comment();
		data->global.GlyphY2 = f->read_byte();
		f->read_comment();
		f->read_byte(); // XOffset
		f->read_comment();
		data->global.XFactor = f->read_byte();
		f->read_comment();
		data->global.YFactor = f->read_byte();
		f->read_comment();
		data->glyph.resize(NumX*NumY);
		for (int i=0;i<NumX*NumY;i++) {
			data->glyph[i].Name = str_m_to_utf8(f->read_str());
			data->glyph[i].Width = MaxGlyphWidth;
			data->glyph[i].X2 = f->read_byte();
			data->glyph[i].X1 = f->read_byte();
		}
		f->read_comment();
		string str = f->read_str();
		data->global.UnknownGlyphNo = 0;
		foreachi(auto &g, data->glyph, i)
			if (g.Name == str)
				data->global.UnknownGlyphNo = i;

	} else if (ffv == 2) {

		f->read_comment();
		data->global.TextureFile = f->read_str();
		f->read_comment();
		int NumGlyphs = f->read_word();
		f->read_comment();
		data->global.GlyphHeight = f->read_byte();
		f->read_comment();
		data->global.GlyphY1 = f->read_byte();
		f->read_comment();
		data->global.GlyphY2 = f->read_byte();
		f->read_comment();
		data->global.XFactor = f->read_byte();
		f->read_comment();
		data->global.YFactor = f->read_byte();
		f->read_comment();
		data->glyph.resize(NumGlyphs);
		for (int i=0;i<NumGlyphs;i++) {
			data->glyph[i].Name = str_m_to_utf8(f->read_str());
			data->glyph[i].Width = f->read_byte();
			data->glyph[i].X1 = f->read_byte();
			data->glyph[i].X2 = f->read_byte();
		}
		f->read_comment();
		string str = f->read_str();
		data->global.UnknownGlyphNo=0;
		foreachi(auto &g, data->glyph, i)
			if (g.Name == str)
				data->global.UnknownGlyphNo = i;
	} else {
		throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 1, 2));
	}

	delete(f);

	if (deep)
		data->UpdateTexture();
}

void FormatFontX::_save(const string &filename, DataFont *data) {

	File *f = FileCreateText(filename);
	f->WriteFileFormatVersion(false, 2);

	f->write_comment("// Texture");
	f->write_str(data->global.TextureFile);
	f->write_comment("// Num Glyphs");
	f->write_word(data->glyph.num);
	f->write_comment("// Glyph Height");
	f->write_byte(data->global.GlyphHeight);
	f->write_comment("// Glyph Y1");
	f->write_byte(data->global.GlyphY1);
	f->write_comment("// Glyph Y2");
	f->write_byte(data->global.GlyphY2);
	f->write_comment("// Scale Factor X");
	f->write_byte(data->global.XFactor);
	f->write_comment("// Scale Factor Y");
	f->write_byte(data->global.YFactor);
	f->write_comment("// Glyphs (Char, Width, X1, X2)");
	foreachi(auto &g, data->glyph, i) {
		f->write_str(g.Name);
		f->write_byte(g.Width);
		f->write_byte(g.X1);
		f->write_byte(g.X2);
	}
	f->write_comment("// Unknown Char");
	f->write_str(data->glyph[data->global.UnknownGlyphNo].Name);

	f->write_str("#");
	delete(f);
}
