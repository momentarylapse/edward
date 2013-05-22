/*
 * LightmapRayTracing.h
 *
 *  Created on: 21.05.2013
 *      Author: michi
 */

#ifndef LIGHTMAPRAYTRACING_H_
#define LIGHTMAPRAYTRACING_H_

#include "Lightmap.h"


class LightmapRayTracing : public Lightmap
{
public:
	LightmapRayTracing(LightmapData *data);
	virtual ~LightmapRayTracing();

	virtual void Compute();
	virtual Histogram GetHistogram();
};

#endif /* LIGHTMAPRAYTRACING_H_ */
