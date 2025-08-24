/*
 * ParserPython.h
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#ifndef PARSERPYTHON_H_
#define PARSERPYTHON_H_

#include "BaseParser.h"

class ParserPython : public Parser {
public:
	ParserPython();

	Array<Markup> create_markup(const string &text, int offset) override;
};

#endif /* PARSERPYTHON_H_ */
