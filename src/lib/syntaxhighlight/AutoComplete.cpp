/*
 * AutoComplete.cpp
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#include "AutoComplete.h"

namespace syntaxhighlight {
void AutoCompleteData::add(const string &name, const string &context) {
	Item i;
	i.name = name;
	i.context = context;
	suggestions.add(i);
}

void AutoCompleteData::append(const AutoCompleteData &d) {
	for (auto &s: d.suggestions)
		suggestions.add(s);
}
}

