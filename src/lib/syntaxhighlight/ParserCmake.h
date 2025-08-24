/*
 * ParserCmake.h
 *
 *  Created on: 30.09.2024
 *      Author: michi
 */

#ifndef PARSERCMAKE_H_
#define PARSERCMAKE_H_

#include "BaseParser.h"

class ParserCmake : public Parser {
public:
	ParserCmake();

	Array<Markup> create_markup(const string &text, int offset) override;
};

#endif /* PARSERCMAKE_H_ */
