/*
 * ActionWorldEditTerrain.cpp
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#include "ActionWorldEditTerrain.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../x/terrain.h"
#include "../../../lib/nix/nix.h"
#include <assert.h>

ActionWorldEditTerrain::ActionWorldEditTerrain(int _index, const WorldEditingTerrain &_data) {
	index = _index;
	data = _data;
}

void ActionWorldEditTerrain::undo(Data *d) {
	execute(d);
}



void *ActionWorldEditTerrain::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->terrains.num);

	Terrain *t = w->terrains[index].terrain;

	// swap
	WorldEditingTerrain old_data;
	old_data.NumX = t->num_x;
	old_data.NumZ = t->num_z;
	old_data.Pattern = t->pattern;
	old_data.MaterialFile = t->material_file;
	old_data.NumTextures = t->material->textures.num;
	for (int i=0;i<t->material->textures.num;i++) {
		old_data.TextureFile[i] = t->texture_file[i];
		old_data.TextureScale[i] = t->texture_scale[i];
	}
	/*t->num_x = data.NumX;
	t->num_z = data.NumZ;
	t->pattern = data.Pattern;*/
	t->material_file = data.MaterialFile;
	//t->material->copy_from(NULL, LoadMaterial(t->material_file), false);
	t->material->textures.resize(data.NumTextures);
	for (int i=0;i<t->material->textures.num;i++) {
		t->texture_file[i] = data.TextureFile[i];
		t->material->textures[i] = nix::Texture::load(t->texture_file[i]);
		t->texture_scale[i] = data.TextureScale[i];
	}

	// update
	/*if (old_data.NumTextures != data.NumTextures){
		delete(t->vertex_buffer);
		t->vertex_buffer = new nix::OldVertexBuffer(data.NumTextures);
	}*/
	t->force_redraw = true;

	data = old_data;

	return NULL;
}


