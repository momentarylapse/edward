/*
 * Format.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Format.h"
#include "../../Session.h"
#include "../../lib/os/msg.h"

FormatError::FormatError(const string &message) : Exception(message) {}





Format::Format(Session *_s, int cat, const string &ext, const string &desc, Flag _flags) {
	session = _s;
	category = cat;
	extension = ext;
	description = desc;
	flags = _flags;
}

Format::~Format() {
}

void Format::warning(const string &message) {
	msg_error(message);
	if (session)
		session->set_message(message);
}

