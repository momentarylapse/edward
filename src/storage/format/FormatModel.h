/*
 * FormatModel.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMODEL_H_
#define SRC_STORAGE_FORMAT_FORMATMODEL_H_

#include "Format.h"
#include "../../mode_model/data/DataModel.h"

class DataModel;

class FormatModel : public TypedFormat<DataModel> {
public:
	explicit FormatModel(Session *s);

	void _load(const Path &filename, DataModel *data, bool deep) override;
	void _save(const Path &filename, DataModel *data) override;

	void _load_old(LegacyFile& lf, DataModel *data, bool deep);
	void _load_v10(LegacyFile& lf, DataModel *data, bool deep);
	void _load_v11(LegacyFile& lf, DataModel *data, bool deep);
	void _load_v11_edit(LegacyFile& lf, DataModel *data, bool deep);
	//void _load_v11_edit_from_file(File *f, DataModel *data);

	void _save_v11(const Path &filename, DataModel *data);
};

#endif /* SRC_STORAGE_FORMAT_FORMATMODEL_H_ */
