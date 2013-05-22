/*
 * Lightmap.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "Lightmap.h"
#include "LightmapData.h"
#include "../../../Edward.h"
#include "../../../Stuff/Progress.h"

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
	ed->progress->StartCancelable(_("berechne Licht"), 0);
	data->AddTextureLevels();
	data->CreateVertices();
	try{
		Compute();
		ed->progress->End();

	}catch(AbortException &e){
		ed->progress->End();
		return;
	}
	RenderTextures();
}

Lightmap::Histogram Lightmap::GetHistogram()
{
	Lightmap::Histogram h;
	return h;
}

Lightmap::Histogram Lightmap::Preview()
{
	ed->progress->StartCancelable(_("berechne Licht"), 0);
	data->AddTextureLevels(false);
	data->CreateVertices();
	try{
		Compute();
	}catch(AbortException &e){
	}
	ed->progress->End();
	return GetHistogram();
}

color Lightmap::RenderVertex(LightmapData::Vertex &v)
{
	return v.rad;
}

void Lightmap::RenderTextures()
{
	ed->progress->StartCancelable(_("berechne Textur"), 0);
	try{
	foreachi(LightmapData::Model &m, data->Models, mid){
		int w = m.tex_width;
		int h = m.tex_height;
		Image im;
		im.Create(w, h, Black);

		foreachi(LightmapData::Vertex &v, data->Vertices, vi){
			if (v.mod_id != mid)
				continue;
			im.SetPixel(v.x, v.y, RenderVertex(v));

			if ((vi & 127) == 0){
				ed->progress->Set(format(_("%d von %d"), vi, data->Vertices.num), (float)vi / (float)data->Vertices.num);
				if (ed->progress->IsCancelled())
					throw Lightmap::AbortException();
			}
		}

		im.Save("new_lightmap.tga");
	}
	}catch(AbortException &e){
	}
	ed->progress->End();
}


