/*
 * FormatModel.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMODEL_H_
#define SRC_STORAGE_FORMAT_FORMATMODEL_H_

#include "Format.h"
#include "../../Data/Model/DataModel.h"

class DataModel;
class File;

class FormatModel : public TypedFormat<DataModel> {
public:
	FormatModel();

	void _load(const string &filename, DataModel *data, bool deep) override;
	void _save(const string &filename, DataModel *data) override;

	void _load_v10(File *f, DataModel *data, bool deep);
	void _load_v11(File *f, DataModel *data, bool deep);
};

#endif /* SRC_STORAGE_FORMAT_FORMATMODEL_H_ */
