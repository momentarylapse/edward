/*
 * ImporterCairo.cpp
 *
 *  Created on: 22.03.2013
 *      Author: michi
 */

#include "ImporterCairo.h"

ImporterCairo::ImporterCairo()
{
}

ImporterCairo::~ImporterCairo()
{
}

bool ImporterCairo::Import(DataFont *f, const string &font_name)
{
	msg_write("import " + font_name);
}

