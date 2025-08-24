/*
 * ParserIni.h
 *
 *  Created on: 2 Oct 2023
 *      Author: michi
 */

#ifndef SRC_PARSER_PARSERINI_H_
#define SRC_PARSER_PARSERINI_H_

#include "BaseParser.h"

class ParserIni : public Parser {
public:
	ParserIni();

	Array<Markup> create_markup(const string &text, int offset) override;


	Array<Markup> create_markup_header(const string& line, int offset);
	Array<Markup> create_markup_key_value(const string& line, int offset);
};

#endif /* SRC_PARSER_PARSERINI_H_ */
