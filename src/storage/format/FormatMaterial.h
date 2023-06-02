/*
 * FormatMaterial.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMATERIAL_H_
#define SRC_STORAGE_FORMAT_FORMATMATERIAL_H_


#include "Format.h"
#include "../../data/material/DataMaterial.h"

class FormatMaterial: public TypedFormat<DataMaterial> {
public:
	FormatMaterial();

	void _load(const Path &filename, DataMaterial *data, bool deep) override;
	void _save(const Path &filename, DataMaterial *data) override;
};

#endif /* SRC_STORAGE_FORMAT_FORMATMATERIAL_H_ */
