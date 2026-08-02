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

namespace syntaxhighlight {


struct CodeContext {
	kaba::Module* module = nullptr;
	const kaba::Class* c = nullptr;
	const kaba::Function* f = nullptr;
	const kaba::Block* b = nullptr;
	int start = -1, end = -1;
};

class ParserKaba : public Parser {
public:
	ParserKaba();

	Array<Label> find_labels(const string &text) override;
	Array<Error> find_errors(const string &text) override;
	Array<Markup> create_markup(const string &text, int offset) override;

	void clear_symbols();
	void prepare_symbols(const string &text, const Path& filename) override;

	AutoCompleteData run_autocomplete(const string &code, const Path &filename, int offset) override;
	base::optional<SymbolInfo> symbol_info(const string& text, int offset, int length) override;

	CodeContext guess_context(int offset);

	string current_code;
	owned<kaba::IContext> context;
	shared<kaba::Module> module;
	Array<Error> errors;

	Array<CodeContext> block_map;
};

}
#endif

