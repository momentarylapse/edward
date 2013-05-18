/*
 * Lightmap.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "Lightmap.h"
#include "LightmapData.h"

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

void Lightmap::Preview()
{
	Compute();
}

