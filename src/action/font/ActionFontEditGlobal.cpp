/*
 * ActionFontEditGlobal.cpp
 *
 *  Created on: 20.03.2013
 *      Author: michi
 */

#include "ActionFontEditGlobal.h"

ActionFontEditGlobal::ActionFontEditGlobal(const DataFont::GlobalData &_data)
{
	data = _data;
}

ActionFontEditGlobal::~ActionFontEditGlobal()
{
}

void *ActionFontEditGlobal::execute(Data *d)
{
	DataFont *f = dynamic_cast<DataFont*>(d);

	DataFont::GlobalData old_data = f->global;

	f->global = data;

	data = old_data;

	return NULL;
}



void ActionFontEditGlobal::undo(Data *d)
{
	execute(d);
}

