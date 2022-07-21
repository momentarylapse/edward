/*
 * FormatModel3ds.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMODEL3DS_H_
#define SRC_STORAGE_FORMAT_FORMATMODEL3DS_H_

#include "Format.h"
#include "../../Data/Model/DataModel.h"

class DataModel;
class BinaryFormatter;

class FormatModel3ds : public TypedFormat<DataModel> {
public:
	FormatModel3ds();

	void _load(const Path &filename, DataModel *data, bool deep) override;
	void _save(const Path &filename, DataModel *data) override;

	void load_mesh(DataModel *m, BinaryFormatter *f, int _length);
};

#endif /* SRC_STORAGE_FORMAT_FORMATMODEL3DS_H_ */
