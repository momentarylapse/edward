/*
 * FormatWorld.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATWORLD_H_
#define SRC_STORAGE_FORMAT_FORMATWORLD_H_

#include "Format.h"
#include "../../Data/World/DataWorld.h"

class FormatWorld: public TypedFormat<DataWorld> {
public:
	FormatWorld();

	void _load(const string &filename, DataWorld *data, bool deep) override;
	void _save(const string &filename, DataWorld *data) override;

	void _load_old(const string &filename, DataWorld *data, bool deep);
	void _load_xml(const string &filename, DataWorld *data, bool deep);
};

#endif /* SRC_STORAGE_FORMAT_FORMATWORLD_H_ */
