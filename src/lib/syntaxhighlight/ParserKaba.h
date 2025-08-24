/*
 * ParserKaba.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#pragma once

#include "BaseParser.h"

#if __has_include("../kaba/kaba.h")
#define SYNTAX_HIGHLIGHT_KABA
#include "../kaba/kaba.h"
#endif

class ParserKaba : public Parser {
public:
	ParserKaba();

	Array<Label> find_labels(const string &text, int offset) override;
	Array<Markup> create_markup(const string &text, int offset) override;

	void clear_symbols();
	void prepare_symbols(const string &text, const Path& filename) override;

	owned<kaba::Context> context;
};

