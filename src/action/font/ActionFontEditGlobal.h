/*
 * ActionFontEditGlobal.h
 *
 *  Created on: 20.03.2013
 *      Author: michi
 */

#ifndef ACTIONFONTEDITGLOBAL_H_
#define ACTIONFONTEDITGLOBAL_H_

#include "../Action.h"
#include "../../data/font/DataFont.h"

class ActionFontEditGlobal : public Action
{
public:
	ActionFontEditGlobal(const DataFont::GlobalData &_data);
	virtual ~ActionFontEditGlobal();
	string name(){	return "FontEditGlobal";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	DataFont::GlobalData data;
};

#endif /* ACTIONFONTEDITGLOBAL_H_ */
