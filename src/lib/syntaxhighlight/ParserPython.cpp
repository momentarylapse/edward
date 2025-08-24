/*
 * ParserPython.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "ParserPython.h"

ParserPython::ParserPython() : Parser("Python") {
	line_comment_begin = "#";
	string_sub_begin = "{";
	string_sub_end = "}";
	keywords.add("enum");
	keywords.add("class");
	keywords.add("import");
	keywords.add("from");
	keywords.add("as");
	keywords.add("def");
	keywords.add("if");
	keywords.add("else");
	keywords.add("elif");
	keywords.add("while");
	keywords.add("for");
	keywords.add("in");
	keywords.add("is");
	keywords.add("return");
	keywords.add("break");
	keywords.add("continue");
	keywords.add("and");
	keywords.add("or");
	keywords.add("not");
	keywords.add("pass");
	keywords.add("try");
	keywords.add("raise");
	keywords.add("except");
	keywords.add("const");
	keywords.add("self");
	keywords.add("with");
	keywords.add("global");
	keywords.add("namespace");
	types.add("set");
	types.add("dict");
	types.add("str");
	types.add("int");
	types.add("float");
	types.add("list");
	types.add("tuple");
	types.add("Exception");
	compiler_functions.add("print");
	compiler_functions.add("len");
	compiler_functions.add("type");
	compiler_functions.add("range");
	compiler_functions.add("enumerate");
	compiler_functions.add("dir");
	compiler_functions.add("zip");
	compiler_functions.add("map");
	compiler_functions.add("max");
	compiler_functions.add("min");
	compiler_functions.add("sum");
	compiler_functions.add("sorted");
	compiler_functions.add("open");
	compiler_functions.add("sin");
	compiler_functions.add("cos");
	compiler_functions.add("sqrt");
	compiler_functions.add("pow");
	compiler_functions.add("exp");
	compiler_functions.add("log");
	constants.add("True");
	constants.add("False");
	constants.add("nil");
	constants.add("None");
}

Array<Markup> ParserPython::create_markup(const string &text, int offset) {
	return create_markup_default(text, offset);
}

