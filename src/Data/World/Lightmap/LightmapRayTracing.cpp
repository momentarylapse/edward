/*
 * LightmapRayTracing.cpp
 *
 *  Created on: 21.05.2013
 *      Author: michi
 */

#include "LightmapRayTracing.h"
#include "../../../Stuff/Progress.h"
#include "../../../Edward.h"

LightmapRayTracing::LightmapRayTracing(LightmapData *_data) :
	Lightmap(_data)
{
}

LightmapRayTracing::~LightmapRayTracing()
{
}

void LightmapRayTracing::Compute()
{
	Array<int> vem;
	foreachi(LightmapData::Vertex &v, data->Vertices, vi){
		v.rad = Black;
		if (v.em.r + v.em.g + v.em.b > 0)
			vem.add(vi);
	}
	foreachi(int vi, vem, ii){
		LightmapData::Vertex &v_a = data->Vertices[vi];
		foreach(LightmapData::Vertex &v_b, data->Vertices){
			if (data->IsVisible(v_a, v_b)){
				v_b.rad = v_a.em;
			}
		}
		ed->progress->Set(format(_("%d von %d"), ii, vem.num), (float)ii / (float)vem.num);
		if (ed->progress->IsCancelled())
			throw AbortException();
	}
}

