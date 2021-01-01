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

	void _load(const Path &filename, DataModel *data, bool deep) override;
	void _save(const Path &filename, DataModel *data) override;

	void _load_v10(File *f, DataModel *data, bool deep);
	void _load_v11(File *f, DataModel *data, bool deep);

	void _save_v11(const Path &filename, DataModel *data);
	void _save_v11_poly(const Path &filename, DataModel *data);
};

#endif /* SRC_STORAGE_FORMAT_FORMATMODEL_H_ */
