/*
 * ParserShader.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#ifndef PARSERSHADER_H_
#define PARSERSHADER_H_

#include "BaseParser.h"

class ParserShader : public Parser {
public:
	ParserShader();

	//Array<Label> FindLabels(SourceView *sv) override;
	Array<Markup> create_markup(const string &text, int offset) override;
};

#endif /* PARSERSHADER_H_ */
