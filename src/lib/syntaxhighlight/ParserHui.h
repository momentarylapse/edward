/*
 * ParserHui.h
 *
 *  Created on: 27.03.2021
 *      Author: michi
 */

#ifndef PARSERHUI_H_
#define PARSERHUI_H_

#include "BaseParser.h"

class ParserHui : public Parser {
public:
	ParserHui();

	Array<Markup> create_markup(const string &text, int offset) override;
};

#endif /* PARSERHUI_H_ */
