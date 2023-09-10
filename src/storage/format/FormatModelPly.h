/*
 * FormatModelPly.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMODELPLY_H_
#define SRC_STORAGE_FORMAT_FORMATMODELPLY_H_

#include "Format.h"
#include "../../data/model/DataModel.h"

class DataModel;

class FormatModelPly : public TypedFormat<DataModel> {
public:
	FormatModelPly(EdwardWindow *ed);

	void _load(const Path &filename, DataModel *data, bool deep) override;
	void _save(const Path &filename, DataModel *data) override;

};

#endif /* SRC_STORAGE_FORMAT_FORMATMODELPLY_H_ */
