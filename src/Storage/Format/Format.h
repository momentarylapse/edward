/*
 * Format.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMAT_H_
#define SRC_STORAGE_FORMAT_FORMAT_H_

#include "../../lib/base/base.h"

class Data;

class FormatError : public Exception {
public:
	FormatError(const string &message);
};

class FormatUnhandledError : public FormatError {
public:
	FormatUnhandledError() : FormatError("") {}
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


	Format(int cat, const string &ext, const string &desc, Flag flags);
	virtual ~Format();

	void warning(const string &message);

	virtual void load(const string &filename, Data *data, bool deep) {}
	virtual void save(const string &filename, Data *data) {}

	string extension;
	string description;
	int category;
	Flag flags;
};



template<class T>
class TypedFormat : public Format {
public:
	TypedFormat(int cat, const string &ext, const string &desc, Flag flags) : Format(cat, ext, desc, flags) {}
	virtual void _load(const string &filename, T *data, bool deep) {}
	virtual void _save(const string &filename, T *data) {}

	void load(const string &filename, Data *data, bool deep) override {
		_load(filename, dynamic_cast<T*>(data), deep);
	}
	void save(const string &filename, Data *data) override {
		_save(filename, dynamic_cast<T*>(data));
	}
};

#endif /* SRC_STORAGE_FORMAT_FORMAT_H_ */
