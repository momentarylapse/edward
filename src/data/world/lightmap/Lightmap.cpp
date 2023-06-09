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
#include "../../../stuff/Progress.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldObject.h"
#include "../../../data/world/WorldTerrain.h"
#include "../../../y/Terrain.h"
#include "../../../y/EngineData.h"
#include "../../../lib/os/filesystem.h"
#include "../../../lib/os/msg.h"
#include "../../../lib/nix/nix.h"
#include "../../../storage/Storage.h"
#include "../../../y/ModelManager.h"


Lightmap::Histogram::Histogram(Array<float> &e)
{
	max = 0;
	for (float ee: e)
		max = ::max(max, ee);
	const int N = 64;
	f.resize(N);
	for (float ee: e)
		if (ee > 0)
			f[ee * (N - 0.1f) / max] += 1;
	normalize();
}

void Lightmap::Histogram::normalize()
{
	float m = 0;
	for (float ff: f)
		m = ::max(m, ff);

	if (m > 0){
		for (float &ff: f)
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
	ed->progress->start_cancelable(_("calculating light"), 0);
	data->AddTextureLevels();
	data->CreateVertices();
	try{
		Compute();
		ed->progress->end();

	}catch(AbortException &e){
		ed->progress->end();
		return false;
	}
	return RenderTextures();
}

Lightmap::Histogram Lightmap::GetHistogram()
{
	Array<float> e;
	for (LightmapData::Vertex &v: data->Vertices)
		e.add((v.rad.r + v.rad.g + v.rad.b) / 3.0f);

	return Lightmap::Histogram(e);
}

bool Lightmap::Preview()
{
	ed->progress->start_cancelable(_("calculating light"), 0);
	data->AddTextureLevels(false);
	data->CreateVertices();
	try{
		Compute();
	}catch(AbortException &e){
		ed->progress->end();
		return false;
	}
	ed->progress->end();
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
			c.add(im.get_pixel(x, y));
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
				im.set_pixel(x, y, c0 * (1.0f / c0.a));
			else
				// no neighbors -> blue
				im.set_pixel(x, y, color(1, 0.3f, 0.3f, 1));
		}
}

bool Lightmap::RenderTextures()
{
	ed->progress->start_cancelable(_("calculating texture"), 0);
	os::fs::create_directory(engine.texture_dir | data->texture_out_dir);
	os::fs::create_directory(engine.object_dir | data->model_out_dir);
	os::fs::create_directory(engine.map_dir | data->model_out_dir);

	PrepareTextureRendering();

	try{
	foreachi(LightmapData::Model &m, data->Models, mid){
		int w = m.tex_width;
		int h = m.tex_height;
		Image im;
		im.create(w, h, color(0,0,0,0));

		foreachi(LightmapData::Vertex &v, data->Vertices, vi){
			if (v.mod_id != mid)
				continue;
			im.set_pixel(v.x, v.y, RenderVertex(v));

			if ((vi & 127) == 0){
				ed->progress->set(format(_("%d of %d"), vi, data->Vertices.num), (float)vi / (float)data->Vertices.num);
				if (ed->progress->is_cancelled())
					throw Lightmap::AbortException();
			}
		}

		m.tex_name = data->texture_out_dir | (i2s(mid) + ".tga");
		fuzzy_image(im);
		im.save(engine.texture_dir | m.tex_name);

		// edit model
		for (ModelMaterial *mat: m.orig->material){
			mat->texture_levels.resize(2);
			mat->texture_levels[1] = new ModelMaterial::TextureLevel();
			mat->texture_levels[1]->filename = m.tex_name;
			mat->col.albedo = Black;
			mat->col.roughness = 0.5f;
			mat->col.emission = Black;
			mat->col.user = true;
		}
		m.new_name = data->model_out_dir | i2s(mid);
		storage->save(engine.object_dir | m.new_name.with(".model"), m.orig);
	}

	foreachi(LightmapData::Terrain &t, data->Terrains, tid){
		int w = t.tex_width;
		int h = t.tex_height;
		Image im;
		im.create(w, h, color(0,0,0,0));

		foreachi(LightmapData::Vertex &v, data->Vertices, vi){
			if (v.ter_id != tid)
				continue;
			im.set_pixel(v.x, v.y, RenderVertex(v));

			if ((vi & 127) == 0){
				ed->progress->set(format(_("%d of %d"), vi, data->Vertices.num), (float)vi / (float)data->Vertices.num);
				if (ed->progress->is_cancelled())
					throw Lightmap::AbortException();
			}
		}

		t.tex_name = data->texture_out_dir | ("t" + i2s(tid) + ".tga");
		fuzzy_image(im);
		im.save(engine.texture_dir | t.tex_name);

		// edit Terrain
		t.orig->texture_file[t.orig->material->textures.num] = t.tex_name;
		t.orig->texture_scale[t.orig->material->textures.num] = vec3(1.0f, 0, 1.0f);
		t.orig->material->textures.add(NULL);
		t.new_name = data->model_out_dir | i2s(tid);
		data->source_world->terrains[tid].save(engine.map_dir | t.new_name.with(".map"));
	}

	CreateNewWorld();
	}catch(AbortException &e){
		ed->progress->end();
		return false;
	}
	ed->progress->end();
	return true;
}

void Lightmap::CreateNewWorld()
{
	DataWorld w;
	w.meta_data = data->source_world->meta_data;
	w.EgoIndex = data->source_world->EgoIndex;
	w.objects = data->source_world->objects;
	w.terrains = data->source_world->terrains;
	for (LightmapData::Model &m: data->Models)
		w.objects[m.object_index].filename = m.new_name;

	storage->save(engine.map_dir | data->new_world_name.with(".world"), &w);
}

