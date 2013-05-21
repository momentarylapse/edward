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
	try{
		Compute();
		ed->progress->End();

		data->AddTextureLevels();

		ed->progress->StartCancelable(_("berechne Textur"), 0);
		RenderToTexture();
	}catch(AbortException &e){
	}
	ed->progress->End();
}

Lightmap::Histogram Lightmap::GetHistogram()
{
	Lightmap::Histogram h;
	return h;
}

Lightmap::Histogram Lightmap::Preview()
{
	ed->progress->StartCancelable(_("berechne Licht"), 0);
	try{
		Compute();
	}catch(AbortException &e){
	}
	ed->progress->End();
	return GetHistogram();
}

