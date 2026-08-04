/*
 * ParserMarkdown.cpp
 *
 *  Created on: 5 Aug 2026
 *      Author: michi
 */

#include "ParserMarkdown.h"
#include <lib/base/iter.h>

namespace syntaxhighlight {

ParserMarkdown::ParserMarkdown() : Parser("Markdown") {
	macro_begin = "???";
	line_comment_begin = "?????";
	multi_comment_begin = "```";
	multi_comment_end = "```";
}

Array<Markup> ParserMarkdown::create_markup(const string &text, int offset) {
	int index0 = offset;

	Array<Markup> markups;
	bool in_code = false;
	for (const auto& s: text.explode("\n")) {
		if (in_code) {
			markups.add({index0, index0 + s.num, MarkupType::COMMENT_LEVEL_1});
			if (s.head(3) == "```")
				in_code = false;
		} else if (s.trim().head(3) == "```") {
			in_code = true;
			markups.add({index0, index0 + s.num, MarkupType::COMMENT_LEVEL_1});
		} else if (s.trim().head(1) == "#") {
			markups.add({index0, index0 + s.num, MarkupType::SPECIAL});
		} else {
			for (int i=0; i<s.num; i++) {
				if (s[i] == '[') {
					int i1 = s.find("](", i);
					if (i1 > i) {
						int i2 = s.find(")", i1);
						if (i2 > i1) {
							markups.add({index0 + i, index0 + i1 + 1, MarkupType::MODIFIER});
							markups.add({index0 + i1 + 1, index0 + i2 + 1, MarkupType::GLOBAL_VARIABLE});
							i = i2 + 1;
						}
					}
				} else if (s[i] == '`') {
					int i1 = s.find("`", i+1);
					if (i1 > i) {
						markups.add({index0 + i, index0 + i1 + 1, MarkupType::STRING});
						i = i1 + 1;
					}
				} else if (i < s.num-1 and s[i] == '_' and s[i+1] == '_') {
					int i1 = s.find("__", i+2);
					if (i1 > i) {
						markups.add({index0 + i, index0 + i1 + 2, MarkupType::COMPILER_FUNCTION});
						i = i1 + 1;
					}
				} else if (i < s.num-1 and s[i] == '*' and s[i+1] == '*') {
					int i1 = s.find("**", i+2);
					if (i1 > i) {
						markups.add({index0 + i, index0 + i1 + 2, MarkupType::COMPILER_FUNCTION});
						i = i1 + 1;
					}
				} else if (s[i] == '_') {
					int i1 = s.find("_", i+1);
					if (i1 > i) {
						markups.add({index0 + i, index0 + i1 + 1, MarkupType::COMPILER_FUNCTION});
						i = i1 + 1;
					}
				}
			}
		}
		index0 += s.num + 1;
	}
	return markups;
}

Array<Parser::Label> ParserMarkdown::find_labels(const string& text) {
	Array<Label> labels;
	bool in_code = false;
	for (const auto& [i, s]: enumerate(text.explode("\n"))) {
		if (in_code) {
			if (s.head(3) == "```")
				in_code = false;
		} else {
			if (s.head(3) == "```") {
				in_code = true;
				continue;
			}
			if (s.trim().head(3) == "###")
				labels.add({"class", s.sub(3), i, 2});
			else if (s.trim().head(3) == "## ")
				labels.add({"class", s.sub(3), i, 1});
			else if (s.trim().head(2) == "# ")
				labels.add({"class", s.sub(2), i, 0});
		}
	}
	return labels;
}
}

