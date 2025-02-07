/*
 * FormatModel3ds.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMODEL3DS_H_
#define SRC_STORAGE_FORMAT_FORMATMODEL3DS_H_

#include "Format.h"
#include "../../mode_model/data/DataModel.h"

class DataModel;
class Stream;

class FormatModel3ds : public TypedFormat<DataModel> {
public:
	explicit FormatModel3ds(Session *s);

	void _load(const Path &filename, DataModel *data, bool deep) override;
	void _save(const Path &filename, DataModel *data) override;

	void load_mesh(DataModel *m, Stream *f, int _length);
};

#endif /* SRC_STORAGE_FORMAT_FORMATMODEL3DS_H_ */
