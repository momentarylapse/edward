/*
 * LightmapRayTracing.cpp
 *
 *  Created on: 21.05.2013
 *      Author: michi
 */

#include "LightmapRayTracing.h"

LightmapRayTracing::LightmapRayTracing(LightmapData *_data) :
	Lightmap(_data)
{
}

LightmapRayTracing::~LightmapRayTracing()
{
}

void LightmapRayTracing::Compute()
{
}

Lightmap::Histogram LightmapRayTracing::GetHistogram()
{
	Lightmap::Histogram h;
	return h;
}

