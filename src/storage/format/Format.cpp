/*
 * Format.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Format.h"
#include "../../EdwardWindow.h"

FormatError::FormatError(const string &message) : Exception(message) {}





Format::Format(EdwardWindow *_ed, int cat, const string &ext, const string &desc, Flag _flags) {
	ed = _ed;
	category = cat;
	extension = ext;
	description = desc;
	flags = _flags;
}

Format::~Format() {
}

void Format::warning(const string &message) {
	msg_error(message);
	if (ed)
		ed->set_message(message);
}

