/*
 * Format.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Format.h"
#include "../../Edward.h"

FormatError::FormatError(const string &message) : Exception(message) {}

Format::Format(int cat, const string &ext, const string &desc) {
	category = cat;
	extension = ext;
	description = desc;
}

Format::~Format() {
}

void Format::warning(const string &message) {
	msg_error(message);
	if (ed)
		ed->set_message(message);
}

