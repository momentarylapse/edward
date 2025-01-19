/*
 * LightmapRadiosity.h
 *
 *  Created on: 22.05.2013
 *      Author: michi
 */

#ifndef LIGHTMAPRADIOSITY_H_
#define LIGHTMAPRADIOSITY_H_

#include "Lightmap.h"


class LightmapRadiosity : public Lightmap
{
public:
	LightmapRadiosity(LightmapData *data);
	virtual ~LightmapRadiosity();

	virtual void Compute();

	void Iterate();
};

#endif /* LIGHTMAPRADIOSITY_H_ */
