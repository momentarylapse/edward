/*
 * Storage.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_STORAGE_H_
#define SRC_STORAGE_STORAGE_H_

#include "Format/Format.h"

class Storage {
public:
	Storage();
	~Storage();

	void load(const string &filename, Data *data, bool deep = true);
	void save(const string &filename, Data *data);

	bool open(Data *data);
	bool save_as(Data *data);

	Array<Format*> formats;
};

extern Storage *storage;

#endif /* SRC_STORAGE_STORAGE_H_ */
