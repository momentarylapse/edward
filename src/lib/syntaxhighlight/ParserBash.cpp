//
// Created by michi on 8/29/26.
//

#include "ParserBash.h"

namespace syntaxhighlight {

ParserBash::ParserBash() : Parser("Bash") {
	line_comment_begin = "#";
	string_sub_begin = "{";
	string_sub_end = "}";
	keywords.add("if");
	keywords.add("then");
	keywords.add("else");
	keywords.add("for");
	keywords.add("while");
	keywords.add("do");
	keywords.add("fi");
	keywords.add("break");
	keywords.add("continue");
	keywords.add("function");
	compiler_functions = {"echo", "mkdir", "exit", "cd", "cp", "rm", "pwd", "git", "cmake", "ninja"};
//	constants.add("True");
//	constants.add("False");
}

Array<Markup> ParserBash::create_markup(const string &text, int offset) {
	return create_markup_default(text, offset);
}

}

