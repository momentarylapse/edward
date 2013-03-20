/*
 * ActionFontEditGlyph.h
 *
 *  Created on: 20.03.2013
 *      Author: michi
 */

#ifndef ACTIONFONTEDITGLYPH_H_
#define ACTIONFONTEDITGLYPH_H_

#include "../Action.h"
#include "../../Data/Font/DataFont.h"

class ActionFontEditGlyph : public Action
{
public:
	ActionFontEditGlyph(int _index, const DataFont::Glyph &_data);
	virtual ~ActionFontEditGlyph();
	string name(){	return "FontEditGlyph";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	DataFont::Glyph data;
};

#endif /* ACTIONFONTEDITGLYPH_H_ */
