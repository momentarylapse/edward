/*
 * AutoComplete.cpp
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#include "AutoComplete.h"

namespace autocomplete {
void Data::add(const string &name, const string &context) {
	Item i;
	i.name = name;
	i.context = context;
	suggestions.add(i);
}

void Data::append(const Data &d) {
	for (auto &s: d.suggestions)
		suggestions.add(s);
}
}

