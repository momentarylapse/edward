/*
 * FormatWorld.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATWORLD_H_
#define SRC_STORAGE_FORMAT_FORMATWORLD_H_

#include "Format.h"
#include "../../data/world/DataWorld.h"

class FormatWorld: public TypedFormat<DataWorld> {
public:
	explicit FormatWorld(Session *s);

	void _load(const Path &filename, DataWorld *data, bool deep) override;
	void _save(const Path &filename, DataWorld *data) override;

	void _load_old(LegacyFile& lf, DataWorld *data, bool deep);
	void _load_xml(const Path &filename, DataWorld *data, bool deep);
};

#endif /* SRC_STORAGE_FORMAT_FORMATWORLD_H_ */
