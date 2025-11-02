/*
 * AutoComplete.h
 *
 *  Created on: 02.08.2018
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>


namespace autocomplete {

struct Item {
	string name, context;
};

struct Data {
	Array<Item> suggestions;
	int offset;
	void add(const string &name, const string &context);
	void append(const Data &d);
};
};
