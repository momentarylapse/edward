/*
 * AutoComplete.h
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>

namespace syntaxhighlight {

struct AutoCompleteData {
	struct Item {
		string name, context;
	};
	Array<Item> suggestions;
	int offset;
	void add(const string &name, const string &context);
	void append(const AutoCompleteData &d);
};
};
