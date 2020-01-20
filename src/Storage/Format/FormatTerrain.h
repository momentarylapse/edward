/*
 * FormatTerrain.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATTERRAIN_H_
#define SRC_STORAGE_FORMAT_FORMATTERRAIN_H_

#include "Format.h"
#include "../../Data/World/DataWorld.h"

class FormatTerrain : public TypedFormat<WorldTerrain> {
public:
	FormatTerrain();

	void _load(const string &filename, WorldTerrain *data, bool deep) override;
	void _save(const string &filename, WorldTerrain *data) override;
};

#endif /* SRC_STORAGE_FORMAT_FORMATTERRAIN_H_ */
