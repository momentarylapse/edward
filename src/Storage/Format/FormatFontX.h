/*
 * FormatFontX.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATFONTX_H_
#define SRC_STORAGE_FORMAT_FORMATFONTX_H_


#include "Format.h"
#include "../../Data/Font/DataFont.h"

class FormatFontX: public TypedFormat<DataFont> {
public:
	FormatFontX();

	void _load(const Path &filename, DataFont *data, bool deep) override;
	void _save(const Path &filename, DataFont *data) override;
};

#endif /* SRC_STORAGE_FORMAT_FORMATFONTX_H_ */
