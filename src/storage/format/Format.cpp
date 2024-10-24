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

Format::~Format() = default;

void Format::warning(const string &message) {
	msg_error(message);
	if (session)
		session->set_message(message);
}


base::optional<LegacyFile> file_get_legacy_header(const Path& filename) {
	LegacyFile lf{};
	lf.f = os::fs::open(filename, "rt");
	lf.filename = filename;
	auto c = lf.f->read_char();
	if (c == 't') {
		lf.binary = false;
		lf.ffv = lf.f->read_int();
	} else if (c == 'b') {
		lf.binary = true;
		delete lf.f;
		lf.f = os::fs::open(filename, "rb");
		lf.f->seek(1);
		lf.ffv = (int)lf.f->read_word();
	}
	if (lf.ffv == 0 or lf.ffv >= 20) {
		delete lf.f;
		return base::None;
	}
	return lf;
}

