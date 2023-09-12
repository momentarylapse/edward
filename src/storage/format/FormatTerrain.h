/*
 * FormatTerrain.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATTERRAIN_H_
#define SRC_STORAGE_FORMAT_FORMATTERRAIN_H_

#include "Format.h"
#include "../../data/world/DataWorld.h"

class FormatTerrain : public TypedFormat<WorldTerrain> {
public:
	FormatTerrain(Session *s);

	void _load(const Path &filename, WorldTerrain *data, bool deep) override;
	void _save(const Path &filename, WorldTerrain *data) override;
};

#endif /* SRC_STORAGE_FORMAT_FORMATTERRAIN_H_ */
