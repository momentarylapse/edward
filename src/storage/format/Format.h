/*
 * Format.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMAT_H_
#define SRC_STORAGE_FORMAT_FORMAT_H_

#include "../../lib/base/base.h"
#include "../../lib/base/optional.h"
#include "../../lib/image/color.h"
#include "../../lib/os/file.h"

class Data;
struct color;
class Path;
class Session;

class FormatError : public Exception {
public:
	explicit FormatError(const string &message);
};

class FormatUnhandledError : public FormatError {
public:
	explicit FormatUnhandledError(const string& ext) : FormatError("format unhandled: " + ext) {}
};

template<class F>
void write_color_rgba(F *f, const color &c) {
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
	f->write_int((int)(c.a * 255.0f));
}

template<class F>
void read_color_rgba(F *f, color &c) {
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
	c.a = (float)f->read_int() / 255.0f;
}

template<class F>
void write_color_argb(F *f, const color &c) {
	f->write_int((int)(c.a * 255.0f));
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
}

template<class F>
void read_color_argb(F *f, color &c) {
	c.a = (float)f->read_int() / 255.0f;
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
}

template<class F>
void write_color_3i(F *f, const color &c) {
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
}

template<class F>
void read_color_3i(F *f, color &c) {
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
	c.a = 1;
}


// file types
enum {
	FD_MODEL,
	FD_TEXTURE,
	FD_SOUND,
	FD_MATERIAL,
	FD_TERRAIN,
	FD_WORLD,
	FD_SHADERFILE,
	FD_FONT,
	FD_SCRIPT,
	FD_CAMERAFLIGHT,
	FD_FILE, // any file
	FD_PROJECT,
	NUM_FDS
};


class Format {
public:
	enum class Flag {
		NONE = 0,
		READ = 1,
		WRITE = 2,
		READ_WRITE = 3,
		CANONICAL = 4,
		CANONICAL_READ_WRITE = 7,
	};


	Format(Session *s, int cat, const string &ext, const string &desc, Flag flags);
	virtual ~Format();

	void warning(const string &message);

	virtual void load(const Path &filename, Data *data, bool deep) {}
	virtual void save(const Path &filename, Data *data) {}

	Session *session;
	string extension;
	string description;
	int category;
	Flag flags;
};



template<class T>
class TypedFormat : public Format {
public:
	TypedFormat(Session *s, int cat, const string &ext, const string &desc, Flag flags) : Format(s, cat, ext, desc, flags) {}
	virtual void _load(const Path &filename, T *data, bool deep) {}
	virtual void _save(const Path &filename, T *data) {}

	void load(const Path &filename, Data *data, bool deep) override {
		_load(filename, reinterpret_cast<T*>(data), deep);
	}
	void save(const Path &filename, Data *data) override {
		_save(filename, reinterpret_cast<T*>(data));
	}
};

struct LegacyFile {
	int ffv;
	bool binary;
	Path filename;
	os::fs::FileStream* f;
};

base::optional<LegacyFile> file_get_legacy_header(const Path& filename);

#endif /* SRC_STORAGE_FORMAT_FORMAT_H_ */
