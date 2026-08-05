/*
 * ParserXml.cpp
 *
 *  Created on: 5 Aug 2026
 *      Author: michi
 */

#include "ParserXml.h"
#include <lib/base/iter.h>

namespace syntaxhighlight {

ParserXml::ParserXml() : Parser("Xml") {
	macro_begin = "???";
	line_comment_begin = "?????";
	multi_comment_begin = "```";
	multi_comment_end = "```";
}

Array<Markup> ParserXml::create_markup(const string &text, int offset) {
	int index0 = offset;

	Array<Markup> markups;
	int last = 0;
	for (const auto& s: text.explode("\n")) {
		for (int i=0; i<s.num; i++) {
			if (s[i] == '<') {
				if (index0 + i > last)
					markups.add({last, index0 + i, MarkupType::NUMBER});
				int i99 = s.find(">", i);
				if (i99 > i) {
					int i1 = s.find(" ", i);
					if (i1 > i and i1 < i99) {
						markups.add({index0 + i, index0 + i1, MarkupType::LOCAL_VARIABLE});
						markups.add({index0 + i1, index0 + i99, MarkupType::MODIFIER});
						markups.add({index0 + i99, index0 + i99 + 1, MarkupType::LOCAL_VARIABLE});
					} else {
						markups.add({index0 + i, index0 + i99 + 1, MarkupType::LOCAL_VARIABLE});
					}
					i = i99;
					last = index0 + i99 + 1;
				}
			}
		}
		index0 += s.num + 1;
	}
	return markups;
}

Array<Parser::Label> ParserXml::find_labels(const string& text) {
	Array<Label> labels;
	return labels;
}
}

