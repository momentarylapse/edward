/*
 * FormatMaterial.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMATERIAL_H_
#define SRC_STORAGE_FORMAT_FORMATMATERIAL_H_


#include "Format.h"
#include "../../Data/Material/DataMaterial.h"

class FormatMaterial: public TypedFormat<DataMaterial> {
public:
	FormatMaterial();

	void _load(const string &filename, DataMaterial *data, bool deep) override;
	void _save(const string &filename, DataMaterial *data) override;
};

#endif /* SRC_STORAGE_FORMAT_FORMATMATERIAL_H_ */
