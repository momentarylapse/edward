/*
 * ParserXml.h
 *
 *  Created on: 5 Aug 2026
 *      Author: michi
 */

#pragma once

#include "BaseParser.h"

namespace syntaxhighlight {

class ParserXml : public Parser {
public:
	ParserXml();

	Array<Markup> create_markup(const string &text, int offset) override;
	Array<Label> find_labels(const string &text) override;
};

}
