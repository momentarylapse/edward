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

#include "LightmapData.h"

class LightmapData;

class Lightmap
{
public:
	Lightmap(LightmapData *data);
	virtual ~Lightmap();

	void Create();
	virtual void Compute(){};
	void RenderTextures();
	virtual color RenderVertex(LightmapData::Vertex &v);

	LightmapData *data;

	struct Histogram
	{
		float max;
		Array<float> f;
		void normalize();
	};
	Histogram Preview();
	virtual Histogram GetHistogram();

	class AbortException
	{};
};

#endif /* LIGHTMAP_H_ */
