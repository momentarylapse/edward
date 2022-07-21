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

class FormatModel : public TypedFormat<DataModel> {
public:
	FormatModel();

	void _load(const Path &filename, DataModel *data, bool deep) override;
	void _save(const Path &filename, DataModel *data) override;

	void _load_old(const Path &filename, DataModel *data, bool deep);
	template<class F>
	void _load_v10(F *f, DataModel *data, bool deep);
	template<class F>
	void _load_v11(F *f, DataModel *data, bool deep);
	template<class F>
	void _load_v11_edit(F *f, DataModel *data, bool deep);
	//void _load_v11_edit_from_file(File *f, DataModel *data);

	void _save_v11(const Path &filename, DataModel *data);
};

#endif /* SRC_STORAGE_FORMAT_FORMATMODEL_H_ */
