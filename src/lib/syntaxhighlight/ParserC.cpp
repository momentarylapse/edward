/*
 * ParserC.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserC.h"

ParserC::ParserC() : Parser("C/C++") {
	macro_begin = "#";
	line_comment_begin = "//";
	multi_comment_begin = "/*";
	multi_comment_end = "*/";
	keywords.add("enum");
	keywords.add("class");
	keywords.add("if");
	keywords.add("else");
	keywords.add("while");
	keywords.add("for");
	keywords.add("return");
	keywords.add("break");
	keywords.add("continue");
	keywords.add("and");
	keywords.add("or");
	keywords.add("new");
	keywords.add("delete");
	keywords.add("this");
	keywords.add("namespace");
	keywords.add("struct");
	keywords.add("template");
	modifiers.add("extern");
	modifiers.add("virtual");
	modifiers.add("static");
	modifiers.add("public");
	modifiers.add("private");
	modifiers.add("protected");
	modifiers.add("const");
	types.add("int");
	types.add("void");
	types.add("float");
	types.add("double");
	types.add("long");
	types.add("char");
	types.add("bool");
	types.add("signed");
	types.add("unsigned");
	types.add("short");
	compiler_functions.add("printf");
	compiler_functions.add("sin");
	compiler_functions.add("cos");
	constants.add("NULL");
	constants.add("true");
	constants.add("false");
}

Array<Markup> ParserC::create_markup(const string &text, int offset) {
	return create_markup_default(text, offset);
}
