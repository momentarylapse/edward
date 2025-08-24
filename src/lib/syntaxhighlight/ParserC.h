/*
 * ParserC.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef PARSERC_H_
#define PARSERC_H_

#include "BaseParser.h"

class ParserC : public Parser {
public:
	ParserC();

	Array<Markup> create_markup(const string &text, int offset) override;
};

#endif /* PARSERC_H_ */
