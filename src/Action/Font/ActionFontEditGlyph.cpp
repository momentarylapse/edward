/*
 * ActionFontEditGlyph.cpp
 *
 *  Created on: 20.03.2013
 *      Author: michi
 */

#include "ActionFontEditGlyph.h"
#include <assert.h>

ActionFontEditGlyph::ActionFontEditGlyph(int _index, const DataFont::Glyph &_data)
{
	index = _index;
	data = _data;
}

ActionFontEditGlyph::~ActionFontEditGlyph()
{
}

void *ActionFontEditGlyph::execute(Data *d)
{
	DataFont *f = dynamic_cast<DataFont*>(d);
	assert(index >= 0);
	assert(index < f->glyph.num);

	DataFont::Glyph old_data = f->glyph[index];

	f->glyph[index] = data;

	data = old_data;

	return NULL;
}



void ActionFontEditGlyph::undo(Data *d)
{
	execute(d);
}

