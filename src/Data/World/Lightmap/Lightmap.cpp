/*
 * Lightmap.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "Lightmap.h"
#include "LightmapData.h"

void Lightmap::Histogram::normalize()
{
	float m = 0;
	foreach(float ff, f)
		m = max(m, ff);

	if (m > 0){
		foreach(float &ff, f)
			ff /= m;
	}
}

Lightmap::Lightmap(LightmapData *_data)
{
	data = _data;
}

Lightmap::~Lightmap()
{
}

void Lightmap::Create()
{
	Compute();
}

Lightmap::Histogram Lightmap::GetHistogram()
{
	Lightmap::Histogram h;
	return h;
}

Lightmap::Histogram Lightmap::Preview()
{
	Compute();
	return GetHistogram();
}

