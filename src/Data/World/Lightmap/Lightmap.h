/*
 * Lightmap.h
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#ifndef LIGHTMAP_H_
#define LIGHTMAP_H_

#include "../../../lib/base/base.h"
#include "../../../lib/math/math.h"

class LightmapData;

class Lightmap
{
public:
	Lightmap(LightmapData *data);
	virtual ~Lightmap();

	void Create();
	void Preview();
	virtual void Compute(){};

	LightmapData *data;

	Array<float> histogram;
};

#endif /* LIGHTMAP_H_ */
