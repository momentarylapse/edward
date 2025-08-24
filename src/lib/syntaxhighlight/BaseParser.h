/*
 * BaseParser.h
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>

class Path;

enum class MarkupType {
	WORD,
	SPECIAL,
	COMPILER_FUNCTION,
	OPERATOR_FUNCTION,
	MODIFIER,
	GLOBAL_VARIABLE,
	TYPE,
	LINE_COMMENT,
	COMMENT_LEVEL_1,
	COMMENT_LEVEL_2,
	MACRO,
	SPACE,
	STRING,
	STRING_SUBSTITUTE,
	OPERATOR,
	NUMBER,
	NUM_TYPES
};

struct Markup {
	int start, end;
	MarkupType type;
};

class Parser {
public:
	explicit Parser(const string &name);
	virtual ~Parser();

	string name;
	string macro_begin;
	string line_comment_begin;
	string multi_comment_begin;
	string multi_comment_end;
	string string_sub_begin;
	string string_sub_end;
	Array<string> keywords;
	Array<string> modifiers;
	Array<string> compiler_functions;
	Array<string> operator_functions;
	Array<string> types;
	Array<string> global_variables;
	Array<string> functions;
	Array<string> constants;


	struct Label {
		string name;
		int line;
		int level;
	};

	virtual void prepare_symbols(const string& text, const Path& filename) {}
	virtual Array<Label> find_labels(const string& text, int offset);
	virtual MarkupType word_type(const string &name);
	virtual Array<Markup> create_markup(const string& text, int offset);

	Array<Markup> create_markup_default(const string& text, int offset);
};

void InitParser();
Parser *GetParser(const Path &filename);


enum {
	CHAR_SPACE,
	CHAR_LETTER,
	CHAR_NUMBER,
	CHAR_SIGN
};

inline int char_type(char c) {
	if ((c >= '0') and (c <= '9'))
		return CHAR_NUMBER;
	if ((c == ' ') or (c == '\n') or (c == '\t'))
		return CHAR_SPACE;
	if (((c >= 'a') and (c <= 'z')) or ((c >= 'A') and (c <= 'Z')) or (c == '_'))
		return CHAR_LETTER;
	return CHAR_SIGN;
}

