/*
 * ActionWorldAddTerrain.cpp
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#include "ActionWorldAddTerrain.h"
#include "../../Data/World/DataWorld.h"
#include <assert.h>

ActionWorldAddTerrain::ActionWorldAddTerrain(const vector &_pos, const string &_filename)
{
	filename = _filename;
	assert(filename.num > 0);
	pos = _pos;
}

ActionWorldAddTerrain::ActionWorldAddTerrain(const vector &_pos, const vector &_size, int _num_x, int _num_z)
{
	pos = _pos;
	size = _size;
	num_x = _num_x;
	num_z = _num_z;
}

ActionWorldAddTerrain::~ActionWorldAddTerrain()
{
}

void ActionWorldAddTerrain::undo(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(w->Terrain.num > 0);

	delete(w->Terrain.back().terrain);
	w->Terrain.pop();
}



void *ActionWorldAddTerrain::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	ModeWorldTerrain t;

	t.Changed = true;
	t.ShowTexture = -1;
	t.is_selected = true;
	t.is_special = false;
	t.pos = pos;

	if (filename == ""){
		t.terrain = new CTerrain();
		t.terrain->pos = pos;
		t.terrain->num_x = num_x;
		t.terrain->num_z = num_z;
		int num = (num_x + 1) * (num_z + 1);
		t.terrain->height.resize(num);
		t.terrain->normal.resize(num);
		t.terrain->vertex.resize(num);
		t.terrain->pattern.x = size.x / num_x;
		t.terrain->pattern.y = 0;
		t.terrain->pattern.z = size.z / num_z;
		t.terrain->num_textures = 1;
		t.terrain->texture[0] = -1;
		t.terrain->texture_scale[0].x = 1.0f / num_x;
		t.terrain->texture_scale[0].y = 0;
		t.terrain->texture_scale[0].z = 1.0f / num_z;
		t.terrain->material = MetaLoadMaterial("");
		for (int x=0;x<num_x/32+1;x++)
			for (int z=0;z<num_z/32+1;z++)
				t.terrain->partition[x][z] = -1;
		t.terrain->vertex_buffer = NixCreateVB(65536);
		t.terrain->Update(-1, -1, -1, -1, TerrainUpdateAll);
		// bounding box
		t.terrain->min = pos;
		t.terrain->max = pos + size;
		t.terrain->force_redraw = true;
	}else
		t.terrain = new CTerrain(filename, pos);

	w->Terrain.add(t);
	return &w->Terrain.back();
}

