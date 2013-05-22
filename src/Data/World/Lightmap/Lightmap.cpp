/*
 * Lightmap.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include <algorithm>
#include "Lightmap.h"
#include "LightmapData.h"
#include "../../../Edward.h"
#include "../../../Stuff/Progress.h"


Lightmap::Histogram::Histogram(Array<float> &e)
{
	max = 0;
	foreach(float ee, e)
		max = max(max, ee);
	const int N = 64;
	f.resize(N);
	foreach(float ee, e)
		if (ee > 0)
			f[ee * (N - 0.1f) / max] += 1;
	normalize();
}

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
	Array<float> e;
	foreach(LightmapData::Vertex &v, data->Vertices)
		e.add((v.rad.r + v.rad.g + v.rad.b) / 3.0f);

	return Lightmap::Histogram(e);
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
	color r = v.rad;
	r.clamp();
	if (data->color_exponent != 1){
		r.r = pow(r.r, data->color_exponent);
		r.g = pow(r.g, data->color_exponent);
		r.b = pow(r.b, data->color_exponent);
	}
	return r;
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


