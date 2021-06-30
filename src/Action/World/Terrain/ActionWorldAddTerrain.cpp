/*
 * ActionWorldAddTerrain.cpp
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#include "ActionWorldAddTerrain.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Data/World/WorldTerrain.h"
#include "../../../y/Terrain.h"
#include "../../../lib/nix/nix.h"
#include <assert.h>

ActionWorldAddTerrain::ActionWorldAddTerrain(const vector &_pos, const Path &_filename) {
	filename = _filename;
	assert(!filename.is_empty());
	pos = _pos;
}

ActionWorldAddTerrain::ActionWorldAddTerrain(const vector &_pos, const vector &_size, int _num_x, int _num_z) {
	pos = _pos;
	size = _size;
	num_x = _num_x;
	num_z = _num_z;
}

void ActionWorldAddTerrain::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(w->terrains.num > 0);

	delete(w->terrains.back().terrain);
	w->terrains.pop();
}



void *ActionWorldAddTerrain::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	WorldTerrain t;

	t.Changed = true;
	t.ShowTexture = -1;
	t.is_selected = true;
	t.is_special = false;
	t.pos = pos;

	if (filename == "") {
		t.terrain = new Terrain();
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
		t.terrain->material->textures.clear();
		t.terrain->material->textures.add(NULL);
		t.terrain->texture_scale[0].x = 1.0f;
		t.terrain->texture_scale[0].y = 0;
		t.terrain->texture_scale[0].z = 1.0f;
		//t.terrain->material->copy_from(NULL, LoadMaterial(""), false);
		for (int x=0;x<num_x/32+1;x++)
			for (int z=0;z<num_z/32+1;z++)
				t.terrain->partition[x][z] = -1;
		t.terrain->vertex_buffer = new nix::VertexBuffer("3f,3f,2f");
		t.terrain->update(-1, -1, -1, -1, TerrainUpdateAll);
		// bounding box
		t.terrain->min = pos;
		t.terrain->max = pos + size;
		t.terrain->force_redraw = true;
	} else {
		t.terrain = new Terrain(filename, pos);
		t.filename = filename;
	}

	w->terrains.add(t);
	return &w->terrains.back();
}


