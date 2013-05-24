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
#include "../../../Data/Model/DataModel.h"
#include "../../../x/model_manager.h"


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

bool Lightmap::Create()
{
	ed->progress->StartCancelable(_("berechne Licht"), 0);
	data->AddTextureLevels();
	data->CreateVertices();
	try{
		Compute();
		ed->progress->End();

	}catch(AbortException &e){
		ed->progress->End();
		return false;
	}
	return RenderTextures();
}

Lightmap::Histogram Lightmap::GetHistogram()
{
	Array<float> e;
	foreach(LightmapData::Vertex &v, data->Vertices)
		e.add((v.rad.r + v.rad.g + v.rad.b) / 3.0f);

	return Lightmap::Histogram(e);
}

bool Lightmap::Preview()
{
	ed->progress->StartCancelable(_("berechne Licht"), 0);
	data->AddTextureLevels(false);
	data->CreateVertices();
	try{
		Compute();
	}catch(AbortException &e){
		ed->progress->End();
		return false;
	}
	ed->progress->End();
	return true;
}

color Lightmap::RenderVertex(LightmapData::Vertex &v)
{
	color r = v.rad;
	r.a = 1;
	r.clamp();
	if (data->color_exponent != 1){
		r.r = pow(r.r, data->color_exponent);
		r.g = pow(r.g, data->color_exponent);
		r.b = pow(r.b, data->color_exponent);
	}
	return r;
}

void fuzzy_image(Image &im)
{
	Array<color> c;
	for (int x=0;x<im.width;x++)
		for (int y=0;y<im.height;y++)
			c.add(im.GetPixel(x, y));
	for (int x=0;x<im.width;x++)
		for (int y=0;y<im.height;y++){
			color c0 = c[y + x * im.height];
			color c1 = c[y + max(x-1,0) * im.height];
			color c2 = c[y + min(x+1,im.width-1) * im.height];
			color c3 = c[max(y-1,0) + x * im.height];
			color c4 = c[min(y+1,im.height-1) + x * im.height];

			// rendered pixel -> blur with neighbors
			// undefined pixel -> copy neighbor
			c0 = c0 + c1 + c2 + c3 + c4;
			if (c0.a > 0)
				im.SetPixel(x, y, c0 * (1.0f / c0.a));
			else
				// no neighbors -> blue
				im.SetPixel(x, y, color(1, 0.3f, 0.3f, 1));
		}
}

bool Lightmap::RenderTextures()
{
	ed->progress->StartCancelable(_("berechne Textur"), 0);
	dir_create(NixTextureDir + data->texture_out_dir);
	dir_create(ObjectDir + data->model_out_dir);
	try{
	foreachi(LightmapData::Model &m, data->Models, mid){
		int w = m.tex_width;
		int h = m.tex_height;
		Image im;
		im.Create(w, h, color(0,0,0,0));

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

		m.tex_name = data->texture_out_dir + i2s(mid) + ".tga";
		fuzzy_image(im);
		im.Save(NixTextureDir + m.tex_name);

		// edit model
		foreach(ModelMaterial &mat, m.orig->Material){
			mat.NumTextures = 2;
			mat.TextureFile[1] = m.tex_name;
			mat.Ambient = White;
			mat.Diffuse = Black;
			mat.Emission = Black;
			mat.UserColor = true;
		}
		m.new_name = data->model_out_dir + i2s(mid);
		m.orig->Save(ObjectDir + m.new_name + ".model");
	}
	}catch(AbortException &e){
		ed->progress->End();
		return false;
	}
	ed->progress->End();
	return true;
}


