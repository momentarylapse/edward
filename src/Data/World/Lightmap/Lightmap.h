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

class Lightmap {
public:
	Lightmap(LightmapData *data);
	virtual ~Lightmap();

	bool Create();
	virtual void Compute(){}
	virtual void PrepareTextureRendering(){}
	bool RenderTextures();
	virtual color RenderVertex(LightmapData::Vertex &v);
	void CreateNewWorld();

	LightmapData *data;

	struct Histogram {
		Histogram(Array<float> &e);
		float max;
		Array<float> f;
		void normalize();
	};
	bool Preview();
	virtual Histogram GetHistogram();

	class AbortException
	{};
};

#endif /* LIGHTMAP_H_ */
