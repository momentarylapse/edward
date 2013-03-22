/*
 * ImporterCairo.h
 *
 *  Created on: 22.03.2013
 *      Author: michi
 */

#ifndef IMPORTERCAIRO_H_
#define IMPORTERCAIRO_H_

#include "../DataFont.h"

class ImporterCairo
{
public:
	ImporterCairo();
	virtual ~ImporterCairo();

	bool Import(DataFont *f, const string &font_name);
	bool TryImport(DataFont *f, const string &font_name, int w, int h);
};

#endif /* IMPORTERCAIRO_H_ */
