/*
 * Format.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "Format.h"
#include "../../Edward.h"

FormatError::FormatError(const string &message) : Exception(message) {}




void write_color_rgba(File *f, const color &c) {
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
	f->write_int((int)(c.a * 255.0f));
}

void read_color_rgba(File *f, color &c) {
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
	c.a = (float)f->read_int() / 255.0f;
}

void write_color_argb(File *f, const color &c) {
	f->write_int((int)(c.a * 255.0f));
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
}

void read_color_argb(File *f, color &c) {
	c.a = (float)f->read_int() / 255.0f;
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
}

void write_color_3i(File *f, const color &c) {
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
}

void read_color_3i(File *f, color &c) {
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
	c.a = 1;
}



Format::Format(int cat, const string &ext, const string &desc, Flag _flags) {
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

